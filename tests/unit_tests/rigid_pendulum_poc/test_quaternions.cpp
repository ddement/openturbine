#include <gtest/gtest.h>

#include "src/rigid_pendulum_poc/quaternion.h"
#include "src/rigid_pendulum_poc/utilities.h"
#include "tests/unit_tests/rigid_pendulum_poc/test_utilities.h"

namespace openturbine::rigid_pendulum::tests {

TEST(QuaternionTest, DefaultConstructor) {
    Quaternion q;
    std::tuple<double, double, double, double> expected = {0., 0., 0., 0.};

    ASSERT_EQ(q.GetComponents(), expected);
}

TEST(QuaternionTest, ConstructorWithProvidedComponents) {
    Quaternion q(1., 2., 3., 4.);
    std::tuple<double, double, double, double> expected = {1., 2., 3., 4.};

    ASSERT_EQ(q.GetComponents(), expected);
}

TEST(QuaternionTest, GetIndividualComponents) {
    Quaternion q(1., 2., 3., 4.);

    ASSERT_EQ(q.GetScalarComponent(), 1.);
    ASSERT_EQ(q.GetXComponent(), 2.);
    ASSERT_EQ(q.GetYComponent(), 3.);
    ASSERT_EQ(q.GetZComponent(), 4.);
}

TEST(QuaternionTest, Length) {
    Quaternion q(1., 2., 3., 4.);

    ASSERT_EQ(q.Length(), std::sqrt(30.));
}

TEST(QuaternionTest, AdditionOfTwoQuaternions) {
    Quaternion q1(1., 2., 3., 4.);
    Quaternion q2(5., 6., 7., 8.);
    Quaternion expected(6., 8., 10., 12.);

    ASSERT_EQ((q1 + q2).GetComponents(), expected.GetComponents());
}

TEST(QuaternionTest, AdditionOfThreeQuaternions) {
    Quaternion q1(1., 2., 3., 4.);
    Quaternion q2(5., 6., 7., 8.);
    Quaternion q3(9., 10., 11., 12.);
    Quaternion expected(15., 18., 21., 24.);

    ASSERT_EQ((q1 + q2 + q3).GetComponents(), expected.GetComponents());
}

TEST(QuaternionTest, SubtractionOfTwoQuaternions) {
    Quaternion q1(1., 2., 3., 4.);
    Quaternion q2(5., 6., 7., 8.);
    Quaternion expected(-4., -4., -4., -4.);

    ASSERT_EQ((q1 - q2).GetComponents(), expected.GetComponents());
}

TEST(QuaternionTest, AdditionAndSubtractionOfThreeQuaternions) {
    Quaternion q1(1., 2., 3., 4.);
    Quaternion q2(5., 6., 7., 8.);
    Quaternion q3(9., 10., 11., 12.);
    Quaternion expected(-3., -2., -1., 0.);

    ASSERT_EQ((q1 + q2 - q3).GetComponents(), expected.GetComponents());
}

TEST(QuaternionTest, MultiplicationOfTwoQuaternions_Set1) {
    Quaternion q1(3., 1., -2., 1.);
    Quaternion q2(2., -1., 2., 3.);
    Quaternion expected(8., -9., -2., 11.);

    ASSERT_EQ((q1 * q2).GetComponents(), expected.GetComponents());
}

TEST(QuaternionTest, MultiplicationOfTwoQuaternions_Set2) {
    Quaternion q1(1., 2., 3., 4.);
    Quaternion q2(5., 6., 7., 8.);
    Quaternion expected(-60., 12., 30., 24.);

    ASSERT_EQ((q1 * q2).GetComponents(), expected.GetComponents());
}

TEST(QuaternionTest, MultiplicationOfQuaternionAndScalar) {
    Quaternion q(1., 2., 3., 4.);
    Quaternion expected(2., 4., 6., 8.);

    ASSERT_EQ((q * 2.).GetComponents(), expected.GetComponents());
}

TEST(QuaternionTest, DivisionOfQuaternionAndScalar) {
    Quaternion q(1., 2., 3., 4.);
    Quaternion expected(0.5, 1., 1.5, 2.);

    ASSERT_EQ((q / 2.).GetComponents(), expected.GetComponents());
}

TEST(QuaternionTest, ExpectNonUnitQuaternion) {
    Quaternion q(1., 2., 3., 4.);

    ASSERT_FALSE(q.IsUnitQuaternion());
}

TEST(QuaternionTest, ExpectUnitQuaternion) {
    double l = std::sqrt(30.);
    Quaternion q(1. / l, 2. / l, 3. / l, 4. / l);

    ASSERT_TRUE(q.IsUnitQuaternion());
}

TEST(QuaternionTest, GetUnitQuaternion) {
    auto sqrt_30 = std::sqrt(30.);
    Quaternion q(1., 2., 3., 4.);
    Quaternion expected(1. / sqrt_30, 2. / sqrt_30, 3. / sqrt_30, 4. / sqrt_30);

    ASSERT_EQ(q.GetUnitQuaternion().GetComponents(), expected.GetComponents());
    ASSERT_TRUE(expected.IsUnitQuaternion());
}

TEST(QuaternionTest, GetConjugate) {
    Quaternion q(1., 2., 3., 4.);
    Quaternion expected(1., -2., -3., -4.);

    ASSERT_EQ(q.GetConjugate().GetComponents(), expected.GetComponents());
}

TEST(QuaternionTest, GetInverse) {
    Quaternion q(1., 2., 3., 4.);
    Quaternion expected(1. / 30., -2. / 30., -3. / 30., -4. / 30.);

    ASSERT_EQ(q.GetInverse().GetComponents(), expected.GetComponents());

    auto q_inv = q.GetInverse();
    ASSERT_TRUE((q * q_inv).IsUnitQuaternion());
}

TEST(QuaternionTest, GetQuaternionFromRotationVector_Set1) {
    Vector rotation_vector{1., 2., 3.};
    auto q = quaternion_from_rotation_vector(rotation_vector);

    // We will use the following quaternion as input in the log conversion
    Quaternion expected(-0.295551, 0.255322, 0.510644, 0.765966);

    ASSERT_NEAR(q.GetScalarComponent(), expected.GetScalarComponent(), 1e-6);
    ASSERT_NEAR(q.GetXComponent(), expected.GetXComponent(), 1e-6);
    ASSERT_NEAR(q.GetYComponent(), expected.GetYComponent(), 1e-6);
    ASSERT_NEAR(q.GetZComponent(), expected.GetZComponent(), 1e-6);
}

TEST(QuaternionTest, GetRotationVectorFromQuaternion_Set1) {
    Quaternion q(-0.295551, 0.255322, 0.510644, 0.765966);
    auto v = rotation_vector_from_quaternion(q);

    // We expect the rotation vector to be same as provided in above exp conversion
    // i.e. {1., 2., 3.}
    Vector expected{1., 2., 3.};

    ASSERT_NEAR(v.GetXComponent(), expected.GetXComponent(), 1e-6);
    ASSERT_NEAR(v.GetYComponent(), expected.GetYComponent(), 1e-6);
    ASSERT_NEAR(v.GetZComponent(), expected.GetZComponent(), 1e-6);
}

TEST(QuaternionTest, GetQuaternionFromRotationVector_Set2) {
    Vector rotation_vector{0., 0., 1.570796};
    auto q = quaternion_from_rotation_vector(rotation_vector);

    // We will use the following quaternion as input in the log conversion
    Quaternion expected(0.707107, 0., 0., 0.707107);

    ASSERT_NEAR(q.GetScalarComponent(), expected.GetScalarComponent(), 1e-6);
    ASSERT_NEAR(q.GetXComponent(), expected.GetXComponent(), 1e-6);
    ASSERT_NEAR(q.GetYComponent(), expected.GetYComponent(), 1e-6);
    ASSERT_NEAR(q.GetZComponent(), expected.GetZComponent(), 1e-6);
}

TEST(QuaternionTest, GetRotationVectorFromQuaternion_Set2) {
    Quaternion q(0.707107, 0., 0., 0.707107);
    auto v = rotation_vector_from_quaternion(q);

    // We expect the rotation vector to be same as provided in above exp conversion
    // i.e. {0., 0., 1.570796}
    Vector expected{0., 0., 1.570796};

    ASSERT_NEAR(v.GetXComponent(), expected.GetXComponent(), 1e-6);
    ASSERT_NEAR(v.GetYComponent(), expected.GetYComponent(), 1e-6);
    ASSERT_NEAR(v.GetZComponent(), expected.GetZComponent(), 1e-6);
}

TEST(QuaternionTest, GetQuaternionFromNullRotationVector) {
    Vector rotation_vector{0., 0., 0.};
    auto q = quaternion_from_rotation_vector(rotation_vector);
    Quaternion expected(1., 0., 0., 0.);

    ASSERT_NEAR(q.GetScalarComponent(), expected.GetScalarComponent(), 1e-6);
    ASSERT_NEAR(q.GetXComponent(), expected.GetXComponent(), 1e-6);
    ASSERT_NEAR(q.GetYComponent(), expected.GetYComponent(), 1e-6);
    ASSERT_NEAR(q.GetZComponent(), expected.GetZComponent(), 1e-6);
}

TEST(QuaternionTest, GetRotationVectorFromNullQuaternion) {
    Quaternion q(1., 0., 0., 0.);
    auto v = rotation_vector_from_quaternion(q);
    Vector expected{0., 0., 0.};

    ASSERT_NEAR(v.GetXComponent(), expected.GetXComponent(), 1e-6);
    ASSERT_NEAR(v.GetYComponent(), expected.GetYComponent(), 1e-6);
    ASSERT_NEAR(v.GetZComponent(), expected.GetZComponent(), 1e-6);
}

TEST(QuaternionTest, QuaternionFromAngleAxis_ZeroAngle) {
    double angle = 0.;
    Vector axis{1., 0., 0.};
    Quaternion q = quaternion_from_angle_axis(angle, axis);

    Quaternion expected(1., 0., 0., 0.);

    ASSERT_NEAR(q.GetScalarComponent(), expected.GetScalarComponent(), 1e-6);
    ASSERT_NEAR(q.GetXComponent(), expected.GetXComponent(), 1e-6);
    ASSERT_NEAR(q.GetYComponent(), expected.GetYComponent(), 1e-6);
    ASSERT_NEAR(q.GetZComponent(), expected.GetZComponent(), 1e-6);
}

TEST(QuaternionTest, AngleAxisFromQuaternion_ZeroAngle) {
    Quaternion q(1., 0., 0., 0.);
    auto [angle, axis] = angle_axis_from_quaternion(q);

    ASSERT_NEAR(angle, 0., 1e-6);
    ASSERT_NEAR(axis.GetXComponent(), 1., 1e-6);
    ASSERT_NEAR(axis.GetYComponent(), 0., 1e-6);
    ASSERT_NEAR(axis.GetZComponent(), 0., 1e-6);
}

TEST(QuaternionTest, QuaternionFromAngleAxis_90Degrees_XAxis) {
    double angle = kPI / 2.;
    Vector axis{1., 0., 0.};
    Quaternion q = quaternion_from_angle_axis(angle, axis);

    Quaternion expected(0.707107, 0.707107, 0., 0.);

    ASSERT_NEAR(q.GetScalarComponent(), expected.GetScalarComponent(), 1e-6);
    ASSERT_NEAR(q.GetXComponent(), expected.GetXComponent(), 1e-6);
    ASSERT_NEAR(q.GetYComponent(), expected.GetYComponent(), 1e-6);
    ASSERT_NEAR(q.GetZComponent(), expected.GetZComponent(), 1e-6);
}

TEST(QuaternionTest, AngleAxisFromQuaternion_90Degrees_XAxis) {
    Quaternion q(0.707107, 0.707107, 0., 0.);
    auto [angle, axis] = angle_axis_from_quaternion(q);

    ASSERT_NEAR(angle, kPI / 2., 1e-6);
    ASSERT_NEAR(axis.GetXComponent(), 1., 1e-6);
    ASSERT_NEAR(axis.GetYComponent(), 0., 1e-6);
    ASSERT_NEAR(axis.GetZComponent(), 0., 1e-6);
}

TEST(QuaternionTest, QuaternionFromAngleAxis_45Degrees_YAxis) {
    double angle = kPI / 4.;
    Vector axis{0., 1., 0.};
    Quaternion q = quaternion_from_angle_axis(angle, axis);

    Quaternion expected(0.923879, 0., 0.382683, 0.);

    ASSERT_NEAR(q.GetScalarComponent(), expected.GetScalarComponent(), 1e-6);
    ASSERT_NEAR(q.GetXComponent(), expected.GetXComponent(), 1e-6);
    ASSERT_NEAR(q.GetYComponent(), expected.GetYComponent(), 1e-6);
    ASSERT_NEAR(q.GetZComponent(), expected.GetZComponent(), 1e-6);
}

TEST(QuaternionTest, AngleAxisFromQuaternion_45Degrees_YAxis) {
    Quaternion q(0.923879, 0., 0.382683, 0.);
    auto [angle, axis] = angle_axis_from_quaternion(q);

    ASSERT_NEAR(angle, kPI / 4., 1e-6);
    ASSERT_NEAR(axis.GetXComponent(), 0., 1e-6);
    ASSERT_NEAR(axis.GetYComponent(), 1., 1e-6);
    ASSERT_NEAR(axis.GetZComponent(), 0., 1e-6);
}

TEST(QuaternionTest, QuaternionFromAngleAxis_60Degrees_ZAxis) {
    double angle = kPI / 3.;
    Vector axis{0., 0., 1.};
    Quaternion q = quaternion_from_angle_axis(angle, axis);

    Quaternion expected(0.866025, 0., 0., 0.5);

    ASSERT_NEAR(q.GetScalarComponent(), expected.GetScalarComponent(), 1e-6);
    ASSERT_NEAR(q.GetXComponent(), expected.GetXComponent(), 1e-6);
    ASSERT_NEAR(q.GetYComponent(), expected.GetYComponent(), 1e-6);
    ASSERT_NEAR(q.GetZComponent(), expected.GetZComponent(), 1e-6);
}

TEST(QuaternionTest, AngleAxisFromQuaternion_60Degrees_ZAxis) {
    Quaternion q(0.866025, 0., 0., 0.5);
    auto [angle, axis] = angle_axis_from_quaternion(q);

    ASSERT_NEAR(angle, kPI / 3., 1e-6);
    ASSERT_NEAR(axis.GetXComponent(), 0., 1e-6);
    ASSERT_NEAR(axis.GetYComponent(), 0., 1e-6);
    ASSERT_NEAR(axis.GetZComponent(), 1., 1e-6);
}

TEST(QuaternionTest, RotateXAXIS_90Degrees_AboutYAxis) {
    double angle = kPI / 2.;
    Vector axis{0., 1., 0.};
    Quaternion q = quaternion_from_angle_axis(angle, axis);

    Vector v{1., 0., 0.};
    Vector rotated = rotate_vector(q, v);
    Vector expected{0., 0., -1.};

    ASSERT_NEAR(rotated.GetXComponent(), expected.GetXComponent(), 1e-6);
    ASSERT_NEAR(rotated.GetYComponent(), expected.GetYComponent(), 1e-6);
    ASSERT_NEAR(rotated.GetZComponent(), expected.GetZComponent(), 1e-6);
}

TEST(QuaternionTest, RotateYAXIS_90Degrees_AboutXAxis) {
    double angle = kPI / 2.;
    Vector axis{1., 0., 0.};
    Quaternion q = quaternion_from_angle_axis(angle, axis);

    Vector v{0., 1., 0.};
    Vector rotated = rotate_vector(q, v);
    Vector expected{0., 0., 1.};

    ASSERT_NEAR(rotated.GetXComponent(), expected.GetXComponent(), 1e-6);
    ASSERT_NEAR(rotated.GetYComponent(), expected.GetYComponent(), 1e-6);
    ASSERT_NEAR(rotated.GetZComponent(), expected.GetZComponent(), 1e-6);
}

TEST(QuaternionTest, RotateZAXIS_90Degrees_AboutXAxis) {
    double angle = kPI / 2.;
    Vector axis{1., 0., 0.};
    Quaternion q = quaternion_from_angle_axis(angle, axis);

    Vector v{0., 0., 1.};
    Vector rotated = rotate_vector(q, v);
    Vector expected{0., -1., 0.};

    ASSERT_NEAR(rotated.GetXComponent(), expected.GetXComponent(), 1e-6);
    ASSERT_NEAR(rotated.GetYComponent(), expected.GetYComponent(), 1e-6);
    ASSERT_NEAR(rotated.GetZComponent(), expected.GetZComponent(), 1e-6);
}

TEST(QuaternionTest, RotateXAXIS_45Degrees_AboutZAxis) {
    double angle = kPI / 4.;
    Vector axis{0., 0., 1.};
    Quaternion q = quaternion_from_angle_axis(angle, axis);

    Vector v{1., 0., 0.};
    Vector rotated = rotate_vector(q, v);
    Vector expected{0.707107, 0.707107, 0.};

    ASSERT_NEAR(rotated.GetXComponent(), expected.GetXComponent(), 1e-6);
    ASSERT_NEAR(rotated.GetYComponent(), expected.GetYComponent(), 1e-6);
    ASSERT_NEAR(rotated.GetZComponent(), expected.GetZComponent(), 1e-6);
}

TEST(QuaternionTest, RotateXAXIS_Neg45Degrees_AboutZAxis) {
    double angle = -kPI / 4.;
    Vector axis{0., 0., 1.};
    Quaternion q = quaternion_from_angle_axis(angle, axis);

    Vector v{1., 0., 0.};
    Vector rotated = rotate_vector(q, v);
    Vector expected{0.707107, -0.707107, 0.};

    ASSERT_NEAR(rotated.GetXComponent(), expected.GetXComponent(), 1e-6);
    ASSERT_NEAR(rotated.GetYComponent(), expected.GetYComponent(), 1e-6);
    ASSERT_NEAR(rotated.GetZComponent(), expected.GetZComponent(), 1e-6);
}

TEST(QuaternionTest, ExpectErrorWhenRotatingVectorWithNonUnitQuaternion) {
    Quaternion q(1., 1., 0., 0.);
    Vector v{1., 0., 0.};

    ASSERT_THROW(rotate_vector(q, v), std::invalid_argument);
}

class QuaternionTest : public ::testing::TestWithParam<std::tuple<Quaternion, RotationMatrix>> {};

TEST_P(QuaternionTest, ConvertQuaternionToRotationMatrix) {
    auto q = std::get<0>(GetParam());  // Provided quaternion
    auto R = std::get<1>(GetParam());  // Expected rotation matrix

    // Convert quaternion to rotation matrix and compare to expected rotation matrix
    auto R_from_q = quaternion_to_rotation_matrix(q);

    EXPECT_NEAR(std::get<0>(R_from_q).GetXComponent(), std::get<0>(R).GetXComponent(), 1e-6);
    EXPECT_NEAR(std::get<0>(R_from_q).GetYComponent(), std::get<0>(R).GetYComponent(), 1e-6);
    EXPECT_NEAR(std::get<0>(R_from_q).GetZComponent(), std::get<0>(R).GetZComponent(), 1e-6);

    EXPECT_NEAR(std::get<1>(R_from_q).GetXComponent(), std::get<1>(R).GetXComponent(), 1e-6);
    EXPECT_NEAR(std::get<1>(R_from_q).GetYComponent(), std::get<1>(R).GetYComponent(), 1e-6);
    EXPECT_NEAR(std::get<1>(R_from_q).GetZComponent(), std::get<1>(R).GetZComponent(), 1e-6);

    EXPECT_NEAR(std::get<2>(R_from_q).GetXComponent(), std::get<2>(R).GetXComponent(), 1e-6);
    EXPECT_NEAR(std::get<2>(R_from_q).GetYComponent(), std::get<2>(R).GetYComponent(), 1e-6);
    EXPECT_NEAR(std::get<2>(R_from_q).GetZComponent(), std::get<2>(R).GetZComponent(), 1e-6);
}

TEST_P(QuaternionTest, RotateSameVectorWithQuaternionAndRotationMatrix) {
    auto q = std::get<0>(GetParam());  // Provided quaternion
    auto R = std::get<1>(GetParam());  // Expected rotation matrix

    // Rotate Vector {1., 1., 1.} by both quaternion and rotation matrix and compare results
    Vector v{1., 1., 1.};
    Vector rotated_by_q = rotate_vector(q, v);
    Vector rotated_by_R = multiply_rotation_matrix_with_vector(R, v);

    EXPECT_NEAR(rotated_by_q.GetXComponent(), rotated_by_R.GetXComponent(), 1e-6);
    EXPECT_NEAR(rotated_by_q.GetYComponent(), rotated_by_R.GetYComponent(), 1e-6);
    EXPECT_NEAR(rotated_by_q.GetZComponent(), rotated_by_R.GetZComponent(), 1e-6);
}

TEST_P(QuaternionTest, ConvertRotationMatrixBackToQuaternion) {
    auto q = std::get<0>(GetParam());  // Provided quaternion
    auto R = std::get<1>(GetParam());  // Expected rotation matrix

    // Convert rotation matrix back to quaternion and compare with original
    auto q_from_R = rotation_matrix_to_quaternion(R);

    EXPECT_NEAR(q_from_R.GetScalarComponent(), q.GetScalarComponent(), 1e-6);
    EXPECT_NEAR(q_from_R.GetXComponent(), q.GetXComponent(), 1e-6);
    EXPECT_NEAR(q_from_R.GetYComponent(), q.GetYComponent(), 1e-6);
    EXPECT_NEAR(q_from_R.GetZComponent(), q.GetZComponent(), 1e-6);
}

TEST_P(QuaternionTest, ConvertRotationMatrixToQuaternion) {
    auto q = std::get<0>(GetParam());  // Expected quaternion
    auto R = std::get<1>(GetParam());  // Provided rotation matrix

    // Convert rotation matrix to quaternion and compare to expected quaternion
    auto q_from_R = rotation_matrix_to_quaternion(R);

    EXPECT_NEAR(q_from_R.GetScalarComponent(), q.GetScalarComponent(), 1e-6);
    EXPECT_NEAR(q_from_R.GetXComponent(), q.GetXComponent(), 1e-6);
    EXPECT_NEAR(q_from_R.GetYComponent(), q.GetYComponent(), 1e-6);
    EXPECT_NEAR(q_from_R.GetZComponent(), q.GetZComponent(), 1e-6);
}

TEST_P(QuaternionTest, RotateSameVectorWithRotationMatrixAndQuaternion) {
    auto q = std::get<0>(GetParam());  // Expected quaternion
    auto R = std::get<1>(GetParam());  // Provided rotation matrix

    // Rotate Vector {1., 1., 1.} by both quaternion and rotation matrix and compare results
    Vector v{1., 1., 1.};
    Vector rotated_by_R = multiply_rotation_matrix_with_vector(R, v);
    Vector rotated_by_q = rotate_vector(q, v);

    EXPECT_NEAR(rotated_by_R.GetXComponent(), rotated_by_q.GetXComponent(), 1e-6);
    EXPECT_NEAR(rotated_by_R.GetYComponent(), rotated_by_q.GetYComponent(), 1e-6);
    EXPECT_NEAR(rotated_by_R.GetZComponent(), rotated_by_q.GetZComponent(), 1e-6);
}

TEST_P(QuaternionTest, ConvertQuaternionBackToRotationMatrix) {
    auto q = std::get<0>(GetParam());  // Expected quaternion
    auto R = std::get<1>(GetParam());  // Provided rotation matrix

    // Convert quaternion back to rotation matrix and compare with original
    auto R_from_q = quaternion_to_rotation_matrix(q);

    EXPECT_NEAR(std::get<0>(R_from_q).GetXComponent(), std::get<0>(R).GetXComponent(), 1e-6);
    EXPECT_NEAR(std::get<0>(R_from_q).GetYComponent(), std::get<0>(R).GetYComponent(), 1e-6);
    EXPECT_NEAR(std::get<0>(R_from_q).GetZComponent(), std::get<0>(R).GetZComponent(), 1e-6);

    EXPECT_NEAR(std::get<1>(R_from_q).GetXComponent(), std::get<1>(R).GetXComponent(), 1e-6);
    EXPECT_NEAR(std::get<1>(R_from_q).GetYComponent(), std::get<1>(R).GetYComponent(), 1e-6);
    EXPECT_NEAR(std::get<1>(R_from_q).GetZComponent(), std::get<1>(R).GetZComponent(), 1e-6);

    EXPECT_NEAR(std::get<2>(R_from_q).GetXComponent(), std::get<2>(R).GetXComponent(), 1e-6);
    EXPECT_NEAR(std::get<2>(R_from_q).GetYComponent(), std::get<2>(R).GetYComponent(), 1e-6);
    EXPECT_NEAR(std::get<2>(R_from_q).GetZComponent(), std::get<2>(R).GetZComponent(), 1e-6);
}

INSTANTIATE_TEST_SUITE_P(
    QuaternionTestSuite, QuaternionTest,
    ::testing::Values(
        // 90 degree rotations about the x, y, and z axes respectively
        std::make_tuple(
            Quaternion{0.707107, 0.707107, 0., 0.},
            RotationMatrix{Vector{1., 0., 0.}, Vector{0., 0., -1.}, Vector{0., 1., 0.}}
        ),
        std::make_tuple(
            Quaternion{0.707107, 0., 0.707107, 0.},
            RotationMatrix{Vector{0., 0., 1.}, Vector{0., 1., 0.}, Vector{-1., 0., 0.}}
        ),
        std::make_tuple(
            Quaternion{0.707107, 0., 0., 0.707107},
            RotationMatrix{Vector{0., -1., 0.}, Vector{1., 0., 0.}, Vector{0., 0., 1.}}
        )
    )
);

}  // namespace openturbine::rigid_pendulum::tests
