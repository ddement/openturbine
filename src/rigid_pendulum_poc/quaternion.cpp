#include "src/rigid_pendulum_poc/quaternion.h"

#include <stdexcept>

namespace openturbine::rigid_pendulum {

Quaternion::Quaternion(double q0, double q1, double q2, double q3)
    : q0_(q0), q1_(q1), q2_(q2), q3_(q3) {
}

bool Quaternion::IsUnitQuaternion() const {
    return close_to(Length(), 1.);
}

Quaternion Quaternion::GetUnitQuaternion() const {
    double length = Length();

    if (close_to(length, 0.)) {
        throw std::runtime_error("Quaternion length is zero, cannot normalize!");
    }

    if (close_to(length, 1.)) {
        return *this;
    }

    return *this / length;
}

Quaternion quaternion_from_rotation_vector(const Vector& vector) {
    auto [v0, v1, v2] = vector.GetComponents();
    double angle = std::sqrt(v0 * v0 + v1 * v1 + v2 * v2);

    // Return the quaternion {1, 0, 0, 0} if provided rotation vector is null
    if (close_to(angle, 0.)) {
        return Quaternion(1.0, 0.0, 0.0, 0.0);
    }

    double sin_angle = std::sin(angle / 2.0);
    double cos_angle = std::cos(angle / 2.0);
    auto factor = sin_angle / angle;

    return Quaternion(cos_angle, v0 * factor, v1 * factor, v2 * factor);
}

Vector rotation_vector_from_quaternion(const Quaternion& quaternion) {
    auto [q0, q1, q2, q3] = quaternion.GetComponents();
    auto sin_angle_squared = q1 * q1 + q2 * q2 + q3 * q3;

    // Return the rotation vector {0, 0, 0} if provided quaternion is null
    if (close_to(sin_angle_squared, 0.)) {
        return Vector{0.0, 0.0, 0.0};
    }

    double sin_angle = std::sqrt(sin_angle_squared);
    double k = 2. * std::atan2(sin_angle, q0) / sin_angle;

    return {q1 * k, q2 * k, q3 * k};
}

Quaternion quaternion_from_angle_axis(double angle, const Vector& axis) {
    auto [v0, v1, v2] = axis.GetComponents();
    double sin_angle = std::sin(angle / 2.0);
    double cos_angle = std::cos(angle / 2.0);

    // We should always get a unit quaternion from the following components
    return Quaternion(cos_angle, v0 * sin_angle, v1 * sin_angle, v2 * sin_angle);
}

std::tuple<double, Vector> angle_axis_from_quaternion(const Quaternion& quaternion) {
    auto [q0, q1, q2, q3] = quaternion.GetComponents();
    double angle = 2. * std::atan2(std::sqrt(q1 * q1 + q2 * q2 + q3 * q3), q0);

    // If angle is null, return the angle 0 and axis {1, 0, 0}
    if (close_to(angle, 0.)) {
        return {0., {1., 0., 0.}};
    }

    angle = wrap_angle_to_pi(angle);
    double k = 1. / std::sqrt(q1 * q1 + q2 * q2 + q3 * q3);
    auto normalized_axis = Vector{q1 * k, q2 * k, q3 * k}.GetUnitVector();

    return {angle, normalized_axis};
}

Vector rotate_vector(const Quaternion& quaternion, const Vector& vector) {
    if (!quaternion.IsUnitQuaternion()) {
        throw std::invalid_argument("Must be a unit quaternion to rotate a vector");
    }

    auto [v0, v1, v2] = vector.GetComponents();
    auto [q0, q1, q2, q3] = quaternion.GetComponents();

    return Vector{
        (q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3) * v0 + 2. * (q1 * q2 - q0 * q3) * v1 +
            2. * (q1 * q3 + q0 * q2) * v2,
        2. * (q1 * q2 + q0 * q3) * v0 + (q0 * q0 - q1 * q1 + q2 * q2 - q3 * q3) * v1 +
            2. * (q2 * q3 - q0 * q1) * v2,
        2. * (q1 * q3 - q0 * q2) * v0 + 2. * (q2 * q3 + q0 * q1) * v1 +
            (q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3) * v2};
}

RotationMatrix quaternion_to_rotation_matrix(const Quaternion& quaternion) {
    if (!quaternion.IsUnitQuaternion()) {
        throw std::invalid_argument(
            "CANNOT convert quaternion to rotation matrix - must be a unit quaternion to rotate a "
            "vector"
        );
    }

    auto [q0, q1, q2, q3] = quaternion.GetComponents();

    return RotationMatrix{
        Vector{
            q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3, 2. * (q1 * q2 - q0 * q3),
            2. * (q1 * q3 + q0 * q2)},
        Vector{
            2. * (q1 * q2 + q0 * q3), q0 * q0 - q1 * q1 + q2 * q2 - q3 * q3,
            2. * (q2 * q3 - q0 * q1)},
        Vector{
            2. * (q1 * q3 - q0 * q2), 2. * (q2 * q3 + q0 * q1),
            q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3}};
}

Quaternion rotation_matrix_to_quaternion(const RotationMatrix& rotation_matrix) {
    auto [m00, m01, m02] = std::get<0>(rotation_matrix).GetComponents();
    auto [m10, m11, m12] = std::get<1>(rotation_matrix).GetComponents();
    auto [m20, m21, m22] = std::get<2>(rotation_matrix).GetComponents();

    auto trace = m00 + m11 + m22;

    if (trace > 0) {
        auto s = 0.5 / std::sqrt(trace + 1.0);
        return Quaternion{0.25 / s, (m21 - m12) * s, (m02 - m20) * s, (m10 - m01) * s};
    } else if (m00 > m11 && m00 > m22) {
        auto s = 2.0 * std::sqrt(1.0 + m00 - m11 - m22);
        return Quaternion((m21 - m12) / s, 0.25 * s, (m01 + m10) / s, (m02 + m20) / s);
    } else if (m11 > m22) {
        auto s = 2.0 * std::sqrt(1.0 + m11 - m00 - m22);
        return Quaternion((m02 - m20) / s, (m01 + m10) / s, 0.25 * s, (m12 + m21) / s);
    } else {
        auto s = 2.0 * std::sqrt(1.0 + m22 - m00 - m11);
        return Quaternion((m10 - m01) / s, (m02 + m20) / s, (m12 + m21) / s, 0.25 * s);
    }
}

}  // namespace openturbine::rigid_pendulum
