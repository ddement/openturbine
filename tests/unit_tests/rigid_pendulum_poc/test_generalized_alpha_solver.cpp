#include <limits>

#include <gtest/gtest.h>

#include "src/rigid_pendulum_poc/solver.h"
#include "src/rigid_pendulum_poc/state.h"
#include "src/rigid_pendulum_poc/time_integrator.h"
#include "tests/unit_tests/rigid_pendulum_poc/test_utilities.h"

namespace openturbine::rigid_pendulum::tests {

TEST(TimeIntegratorTest, CreateDefaultTimeIntegrator) {
    auto time_integrator = GeneralizedAlphaTimeIntegrator();

    EXPECT_EQ(time_integrator.GetInitialTime(), 0.);
    EXPECT_EQ(time_integrator.GetCurrentTime(), 0.);
    EXPECT_EQ(time_integrator.GetTimeStep(), 1.);
    EXPECT_EQ(time_integrator.GetNumberOfSteps(), 1);
}

TEST(TimeIntegratorTest, CreateTimeIntegrator) {
    auto time_integrator = GeneralizedAlphaTimeIntegrator(1., 0.01, 10);

    EXPECT_EQ(time_integrator.GetInitialTime(), 1.);
    EXPECT_EQ(time_integrator.GetCurrentTime(), 1.);
    EXPECT_EQ(time_integrator.GetTimeStep(), 0.01);
    EXPECT_EQ(time_integrator.GetNumberOfSteps(), 10);
}

TEST(TimeIntegratorTest, AdvanceAnalysisTime) {
    auto time_integrator = GeneralizedAlphaTimeIntegrator();

    EXPECT_EQ(time_integrator.GetCurrentTime(), 0.);

    time_integrator.AdvanceTimeStep();
    EXPECT_EQ(time_integrator.GetCurrentTime(), 1.);
}

TEST(StateTest, CreateDefaultState) {
    auto state = State();

    expect_kokkos_view_1D_equal(state.GetGeneralizedCoordinates(), {0.});
    expect_kokkos_view_1D_equal(state.GetGeneralizedVelocity(), {0.});
    expect_kokkos_view_1D_equal(state.GetGeneralizedAcceleration(), {0.});
    expect_kokkos_view_1D_equal(state.GetAlgorithmicAcceleration(), {0.});
}

TEST(StateTest, CreateState) {
    auto v = create_vector({1., 2., 3.});
    auto state = State(v, v, v, v);

    expect_kokkos_view_1D_equal(state.GetGeneralizedCoordinates(), {1., 2., 3.});
    expect_kokkos_view_1D_equal(state.GetGeneralizedVelocity(), {1., 2., 3.});
    expect_kokkos_view_1D_equal(state.GetGeneralizedAcceleration(), {1., 2., 3.});
    expect_kokkos_view_1D_equal(state.GetAlgorithmicAcceleration(), {1., 2., 3.});
}

TEST(TimeIntegratorTest, AdvanceAnalysisTimeByNumberOfSteps) {
    auto time_integrator = GeneralizedAlphaTimeIntegrator(0., 1.0, 10);

    EXPECT_EQ(time_integrator.GetCurrentTime(), 0.);

    auto initial_state = State();
    time_integrator.Integrate(initial_state);

    EXPECT_EQ(time_integrator.GetCurrentTime(), 10.0);
}

TEST(TimeIntegratorTest, GetHistoryOfStatesFromTimeIntegrator) {
    auto time_integrator = GeneralizedAlphaTimeIntegrator(0., 0.10, 17);

    EXPECT_EQ(time_integrator.GetCurrentTime(), 0.);

    auto initial_state = State();
    auto state_history = time_integrator.Integrate(initial_state);

    EXPECT_NEAR(time_integrator.GetCurrentTime(), 1.70, 10 * std::numeric_limits<double>::epsilon());
    EXPECT_EQ(state_history.size(), 18);
}

TEST(TimeIntegratorTest, LinearSolutionWithZeroAcceleration) {
    auto initial_state = State();
    auto time_integrator = GeneralizedAlphaTimeIntegrator(0., 1., 1);
    auto linear_update = time_integrator.UpdateLinearSolution(initial_state);

    expect_kokkos_view_1D_equal(linear_update.GetGeneralizedCoordinates(), {0.});
    expect_kokkos_view_1D_equal(linear_update.GetGeneralizedVelocity(), {0.});
    expect_kokkos_view_1D_equal(linear_update.GetGeneralizedAcceleration(), {0.});
}

TEST(TimeIntegratorTest, LinearSolutionWithNonZeroAcceleration) {
    auto v = create_vector({1., 2., 3.});
    auto initial_state = State(v, v, v, v);
    auto time_integrator = GeneralizedAlphaTimeIntegrator(0., 1., 1);
    auto linear_update = time_integrator.UpdateLinearSolution(initial_state);

    expect_kokkos_view_1D_equal(linear_update.GetGeneralizedCoordinates(), {2.25, 4.5, 6.75});
    expect_kokkos_view_1D_equal(linear_update.GetGeneralizedVelocity(), {1.5, 3., 4.5});
    expect_kokkos_view_1D_equal(linear_update.GetGeneralizedAcceleration(), {0., 0., 0.});
}

TEST(TimeIntegratorTest, TotalNumberOfIterationsInNonLinearSolution) {
    auto time_integrator = GeneralizedAlphaTimeIntegrator(0., 1.0, 10);

    EXPECT_EQ(time_integrator.GetNumberOfIterations(), 0);
    EXPECT_EQ(time_integrator.GetTotalNumberOfIterations(), 0);

    auto initial_state = State();
    time_integrator.Integrate(initial_state);

    EXPECT_LE(time_integrator.GetNumberOfIterations(), time_integrator.GetMaxIterations());
    EXPECT_LE(
        time_integrator.GetTotalNumberOfIterations(),
        time_integrator.GetNumberOfSteps() * time_integrator.GetMaxIterations()
    );
}

TEST(TimeIntegratorTest, ExpectConvergedSolution) {
    auto residual_force = create_vector({1.e-7, 2.e-7, 3.e-7});
    auto incremental_force = create_vector({1., 2., 3.});
    auto time_integrator = GeneralizedAlphaTimeIntegrator(0., 1., 1);
    auto converged = time_integrator.CheckConvergence(residual_force, incremental_force);

    EXPECT_TRUE(converged);
}

TEST(TimeIntegratorTest, ExpectNonConvergedSolution) {
    auto residual_force = create_vector({1.e-3, 2.e-3, 3.e-3});
    auto incremental_force = create_vector({1., 2., 3.});
    auto time_integrator = GeneralizedAlphaTimeIntegrator(0., 1., 1);
    auto converged = time_integrator.CheckConvergence(residual_force, incremental_force);

    EXPECT_FALSE(converged);
}

TEST(GeneralizedAlphaTimeIntegratorTest, ConstructorWithInvalidAlphaF) {
    EXPECT_THROW(
        GeneralizedAlphaTimeIntegrator(0., 1., 1, 1.1, 0.5, 0.25, 0.5, 10), std::invalid_argument
    );
}

TEST(GeneralizedAlphaTimeIntegratorTest, ConstructorWithInvalidAlphaM) {
    EXPECT_THROW(
        GeneralizedAlphaTimeIntegrator(0., 1., 1, 0.5, 1.1, 0.25, 0.5, 10), std::invalid_argument
    );
}

TEST(GeneralizedAlphaTimeIntegratorTest, ConstructorWithInvalidBeta) {
    EXPECT_THROW(
        GeneralizedAlphaTimeIntegrator(0., 1., 1, 0.5, 0.5, 0.75, 0.5, 10), std::invalid_argument
    );
}

TEST(GeneralizedAlphaTimeIntegratorTest, ConstructorWithInvalidGamma) {
    EXPECT_THROW(
        GeneralizedAlphaTimeIntegrator(0., 1., 1, 0.5, 0.5, 0.25, 1.1, 10), std::invalid_argument
    );
}

TEST(GeneralizedAlphaTimeIntegratorTest, ConstructorWithInvalidNumberOfSteps) {
    EXPECT_THROW(
        GeneralizedAlphaTimeIntegrator(0., 1., 1, 0.5, 0.5, 0.25, 0.5, 0), std::invalid_argument
    );
}

TEST(GeneralizedAlphaTimeIntegratorTest, GetDefaultGAConstants) {
    auto time_integrator = GeneralizedAlphaTimeIntegrator();

    EXPECT_EQ(time_integrator.GetAlphaF(), 0.5);
    EXPECT_EQ(time_integrator.GetAlphaM(), 0.5);
    EXPECT_EQ(time_integrator.GetBeta(), 0.25);
    EXPECT_EQ(time_integrator.GetGamma(), 0.5);
    EXPECT_EQ(time_integrator.GetMaxIterations(), 10);
}

TEST(GeneralizedAlphaTimeIntegratorTest, GetSuppliedGAConstants) {
    auto time_integrator = GeneralizedAlphaTimeIntegrator(0., 1., 1, 0.11, 0.29, 0.47, 0.93, 17);

    EXPECT_EQ(time_integrator.GetAlphaF(), 0.11);
    EXPECT_EQ(time_integrator.GetAlphaM(), 0.29);
    EXPECT_EQ(time_integrator.GetBeta(), 0.47);
    EXPECT_EQ(time_integrator.GetGamma(), 0.93);
    EXPECT_EQ(time_integrator.GetMaxIterations(), 17);
}

TEST(TimeIntegratorTest, AlphaStepSolutionAfterOneIncWithZeroAcceleration) {
    auto initial_state = State();
    auto time_integrator = GeneralizedAlphaTimeIntegrator(0., 1., 1, 0., 0., 0.5, 1., 1);

    EXPECT_EQ(time_integrator.GetNumberOfIterations(), 0);
    EXPECT_EQ(time_integrator.GetTotalNumberOfIterations(), 0);

    auto results = time_integrator.Integrate(initial_state);

    EXPECT_EQ(time_integrator.GetNumberOfIterations(), 1);
    EXPECT_EQ(time_integrator.GetTotalNumberOfIterations(), 1);

    auto final_state = results.back();

    // We expect the final state to contain the following values after one increment
    expect_kokkos_view_1D_equal(final_state.GetGeneralizedCoordinates(), {1.});
    expect_kokkos_view_1D_equal(final_state.GetGeneralizedVelocity(), {2.});
    expect_kokkos_view_1D_equal(final_state.GetGeneralizedAcceleration(), {2.});
    expect_kokkos_view_1D_equal(final_state.GetAlgorithmicAcceleration(), {2.});
}

TEST(TimeIntegratorTest, AlphaStepSolutionAfterTwoIncsWithZeroAcceleration) {
    auto initial_state = State();
    auto time_integrator = GeneralizedAlphaTimeIntegrator(0., 1., 1, 0., 0., 0.5, 1., 2);
    auto results = time_integrator.Integrate(initial_state);

    EXPECT_EQ(time_integrator.GetNumberOfIterations(), 2);
    EXPECT_EQ(time_integrator.GetTotalNumberOfIterations(), 2);

    auto final_state = results.back();

    // We expect the final state to contain the following values after two increments
    expect_kokkos_view_1D_equal(final_state.GetGeneralizedCoordinates(), {2.});
    expect_kokkos_view_1D_equal(final_state.GetGeneralizedVelocity(), {4.});
    expect_kokkos_view_1D_equal(final_state.GetGeneralizedAcceleration(), {4.});
    expect_kokkos_view_1D_equal(final_state.GetAlgorithmicAcceleration(), {4.});
}

}  // namespace openturbine::rigid_pendulum::tests
