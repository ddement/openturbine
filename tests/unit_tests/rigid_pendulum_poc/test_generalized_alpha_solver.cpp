#include <limits>

#include <gtest/gtest.h>

#include "src/rigid_pendulum_poc/generalized_alpha_time_integrator.h"
#include "tests/unit_tests/rigid_pendulum_poc/test_utilities.h"

namespace openturbine::rigid_pendulum::tests {

TEST(ProblemTypeTest, DefaultValue) {
    auto problem_type = ProblemType();

    EXPECT_EQ(problem_type, ProblemType::kRigidBody);
}

TEST(TimeIntegratorTest, GetTimeIntegratorType) {
    auto time_integrator =
        GeneralizedAlphaTimeIntegrator(0.5, 0.5, 0.25, 0.5, TimeStepper(0., 1.0, 10));

    EXPECT_EQ(time_integrator.GetType(), TimeIntegratorType::kGENERALIZED_ALPHA);
}

TEST(TimeIntegratorTest, GetTimeIntegratorProblemType) {
    auto time_integrator =
        GeneralizedAlphaTimeIntegrator(0.5, 0.5, 0.25, 0.5, TimeStepper(0., 1.0, 10));

    EXPECT_EQ(time_integrator.GetProblemType(), ProblemType::kRigidBody);
}

TEST(TimeIntegratorTest, AdvanceAnalysisTimeByNumberOfSteps) {
    auto time_integrator =
        GeneralizedAlphaTimeIntegrator(0.5, 0.5, 0.25, 0.5, TimeStepper(0., 1.0, 10));

    EXPECT_EQ(time_integrator.GetTimeStepper().GetCurrentTime(), 0.);

    auto initial_state = State();
    auto mass_matrix = MassMatrix();
    auto gen_forces = GeneralizedForces();
    auto lagrange_mults = HostView1D();
    time_integrator.Integrate(initial_state, mass_matrix, gen_forces, lagrange_mults);

    EXPECT_EQ(time_integrator.GetTimeStepper().GetCurrentTime(), 10.0);
}

TEST(TimeIntegratorTest, GetHistoryOfStatesFromTimeIntegrator) {
    auto time_integrator =
        GeneralizedAlphaTimeIntegrator(0.5, 0.5, 0.25, 0.5, TimeStepper(0., 0.1, 17));

    EXPECT_EQ(time_integrator.GetTimeStepper().GetCurrentTime(), 0.);

    auto initial_state = State();
    auto mass_matrix = MassMatrix();
    auto gen_forces = GeneralizedForces();
    auto lagrange_mults = HostView1D();
    auto state_history =
        time_integrator.Integrate(initial_state, mass_matrix, gen_forces, lagrange_mults);

    EXPECT_NEAR(
        time_integrator.GetTimeStepper().GetCurrentTime(), 1.70,
        10 * std::numeric_limits<double>::epsilon()
    );
    EXPECT_EQ(state_history.size(), 18);
}

TEST(TimeIntegratorTest, TotalNumberOfIterationsInNonLinearSolution) {
    auto time_integrator =
        GeneralizedAlphaTimeIntegrator(0.5, 0.5, 0.25, 0.5, TimeStepper(0., 1., 10));

    EXPECT_EQ(time_integrator.GetTimeStepper().GetNumberOfIterations(), 0);
    EXPECT_EQ(time_integrator.GetTimeStepper().GetTotalNumberOfIterations(), 0);

    auto initial_state = State();
    auto mass_matrix = MassMatrix();
    auto gen_forces = GeneralizedForces();
    auto lagrange_mults = HostView1D();
    time_integrator.Integrate(initial_state, mass_matrix, gen_forces, lagrange_mults);

    EXPECT_LE(
        time_integrator.GetTimeStepper().GetNumberOfIterations(),
        time_integrator.GetTimeStepper().GetMaximumNumberOfIterations()
    );
    EXPECT_LE(
        time_integrator.GetTimeStepper().GetTotalNumberOfIterations(),
        time_integrator.GetTimeStepper().GetNumberOfSteps() *
            time_integrator.GetTimeStepper().GetMaximumNumberOfIterations()
    );
}

TEST(TimeIntegratorTest, TestUpdateGeneralizedCoordinates) {
    auto time_integrator =
        GeneralizedAlphaTimeIntegrator(0.5, 0.5, 0.25, 0.5, TimeStepper(0., 1.0, 10));

    auto gen_coords = create_vector({0., -1., 0., 1., 0., 0., 0.});
    auto delta_gen_coords = create_vector({1., 1., 1., 1., 2., 3.});
    auto gen_coords_next =
        time_integrator.UpdateGeneralizedCoordinates(gen_coords, delta_gen_coords);

    Vector r1{0., -1., 0.};
    Vector r2{1., 1., 1.};
    Vector position = r1 + r2;

    Quaternion q1{1., 0., 0., 0.};
    Vector rotation_vector{1., 2., 3.};
    auto q2 = quaternion_from_rotation_vector(rotation_vector);
    Quaternion orientation = q1 * q2;

    expect_kokkos_view_1D_equal(
        gen_coords_next,
        {
            position.GetXComponent(),          // component 1
            position.GetYComponent(),          // component 2
            position.GetZComponent(),          // component 3
            orientation.GetScalarComponent(),  // component 4
            orientation.GetXComponent(),       // component 5
            orientation.GetYComponent(),       // component 6
            orientation.GetZComponent()        // component 7
        }
    );
}

TEST(TimeIntegratorTest, ExpectConvergedSolution) {
    auto tol = GeneralizedAlphaTimeIntegrator::kCONVERGENCETOLERANCE;
    auto residual = create_vector({tol * 1e-1, tol * 2e-1, tol * 3e-1});
    auto time_integrator = GeneralizedAlphaTimeIntegrator();
    auto converged = time_integrator.CheckConvergence(residual);

    EXPECT_TRUE(converged);
}

TEST(TimeIntegratorTest, ExpectNonConvergedSolution) {
    auto tol = GeneralizedAlphaTimeIntegrator::kCONVERGENCETOLERANCE;
    auto residual = create_vector({tol * 1e1, tol * 2e1, tol * 3e1});
    auto time_integrator = GeneralizedAlphaTimeIntegrator();
    auto converged = time_integrator.CheckConvergence(residual);

    EXPECT_FALSE(converged);
}

TEST(GeneralizedAlphaTimeIntegratorTest, ConstructorWithInvalidAlphaF) {
    EXPECT_THROW(GeneralizedAlphaTimeIntegrator(1.1, 0.5, 0.25, 0.5), std::invalid_argument);
}

TEST(GeneralizedAlphaTimeIntegratorTest, ConstructorWithInvalidAlphaM) {
    EXPECT_THROW(GeneralizedAlphaTimeIntegrator(0.5, 1.1, 0.25, 0.5), std::invalid_argument);
}

TEST(GeneralizedAlphaTimeIntegratorTest, ConstructorWithInvalidBeta) {
    EXPECT_THROW(GeneralizedAlphaTimeIntegrator(0.5, 0.5, 0.75, 0.5), std::invalid_argument);
}

TEST(GeneralizedAlphaTimeIntegratorTest, ConstructorWithInvalidGamma) {
    EXPECT_THROW(GeneralizedAlphaTimeIntegrator(0.5, 0.5, 0.25, 1.1), std::invalid_argument);
}

TEST(GeneralizedAlphaTimeIntegratorTest, GetDefaultGAConstants) {
    auto time_integrator = GeneralizedAlphaTimeIntegrator();

    EXPECT_EQ(time_integrator.GetAlphaF(), 0.5);
    EXPECT_EQ(time_integrator.GetAlphaM(), 0.5);
    EXPECT_EQ(time_integrator.GetBeta(), 0.25);
    EXPECT_EQ(time_integrator.GetGamma(), 0.5);
}

TEST(GeneralizedAlphaTimeIntegratorTest, GetSuppliedGAConstants) {
    auto time_integrator = GeneralizedAlphaTimeIntegrator(0.11, 0.29, 0.47, 0.93);

    EXPECT_EQ(time_integrator.GetAlphaF(), 0.11);
    EXPECT_EQ(time_integrator.GetAlphaM(), 0.29);
    EXPECT_EQ(time_integrator.GetBeta(), 0.47);
    EXPECT_EQ(time_integrator.GetGamma(), 0.93);
}

TEST(TimeIntegratorTest, AlphaStepSolutionAfterOneIncWithZeroAcceleration) {
    auto time_integrator =
        GeneralizedAlphaTimeIntegrator(0., 0., 0.5, 1., TimeStepper(0., 1., 1, 1));

    EXPECT_EQ(time_integrator.GetTimeStepper().GetNumberOfIterations(), 0);
    EXPECT_EQ(time_integrator.GetTimeStepper().GetTotalNumberOfIterations(), 0);

    auto initial_state = State();
    auto mass_matrix = MassMatrix();
    auto gen_forces = GeneralizedForces();
    auto lagrange_mults = HostView1D();
    auto results = time_integrator.Integrate(initial_state, mass_matrix, gen_forces, lagrange_mults);

    EXPECT_EQ(time_integrator.GetTimeStepper().GetNumberOfIterations(), 1);
    EXPECT_EQ(time_integrator.GetTimeStepper().GetTotalNumberOfIterations(), 1);

    auto final_state = results.back();

    // We expect the final state to contain the following values after one increment
    // via hand calculations
    expect_kokkos_view_1D_equal(final_state.GetGeneralizedCoordinates(), {0.});
    expect_kokkos_view_1D_equal(final_state.GetVelocity(), {-2.});
    expect_kokkos_view_1D_equal(final_state.GetAcceleration(), {-2.});
    expect_kokkos_view_1D_equal(final_state.GetAlgorithmicAcceleration(), {-2.});
}

TEST(TimeIntegratorTest, AlphaStepSolutionAfterTwoIncsWithZeroAcceleration) {
    auto time_integrator =
        GeneralizedAlphaTimeIntegrator(0., 0., 0.5, 1., TimeStepper(0., 1., 1, 2));
    auto initial_state = State();
    auto mass_matrix = MassMatrix();
    auto gen_forces = GeneralizedForces();
    auto lagrange_mults = HostView1D();
    auto results = time_integrator.Integrate(initial_state, mass_matrix, gen_forces, lagrange_mults);

    EXPECT_EQ(time_integrator.GetTimeStepper().GetNumberOfIterations(), 2);
    EXPECT_EQ(time_integrator.GetTimeStepper().GetTotalNumberOfIterations(), 2);

    auto final_state = results.back();

    // We expect the final state to contain the following values after two increments
    // via hand calculations
    expect_kokkos_view_1D_equal(final_state.GetGeneralizedCoordinates(), {-1.});
    expect_kokkos_view_1D_equal(final_state.GetVelocity(), {-4.});
    expect_kokkos_view_1D_equal(final_state.GetAcceleration(), {-4.});
    expect_kokkos_view_1D_equal(final_state.GetAlgorithmicAcceleration(), {-4.});
}

TEST(TimeIntegratorTest, AlphaStepSolutionAfterOneIncWithNonZeroAccelerationVector) {
    auto time_integrator =
        GeneralizedAlphaTimeIntegrator(0., 0., 0.5, 1., TimeStepper(0., 1., 1, 1));
    auto v = create_vector({1., 2., 3.});
    auto initial_state = State(v, v, v, v);
    auto mass_matrix = MassMatrix();
    auto gen_forces = GeneralizedForces();
    auto lagrange_mults = HostView1D(v);
    auto results = time_integrator.Integrate(initial_state, mass_matrix, gen_forces, lagrange_mults);

    EXPECT_EQ(time_integrator.GetTimeStepper().GetNumberOfIterations(), 1);
    EXPECT_EQ(time_integrator.GetTimeStepper().GetTotalNumberOfIterations(), 1);

    auto final_state = results.back();

    // We expect the final state to contain the following values after one increment
    // via hand calculations
    expect_kokkos_view_1D_equal(final_state.GetGeneralizedCoordinates(), {2., 4., 6.});
    expect_kokkos_view_1D_equal(final_state.GetVelocity(), {-1., 0., 1.});
    expect_kokkos_view_1D_equal(final_state.GetAcceleration(), {-2., -2., -2.});
    expect_kokkos_view_1D_equal(final_state.GetAlgorithmicAcceleration(), {-2., -2., -2.});
}

}  // namespace openturbine::rigid_pendulum::tests
