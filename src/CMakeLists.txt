
set(Kokkos_DIR "$ENV{Kokkos_ROOT}" CACHE STRING "Kokkos root directory")
find_package(Kokkos REQUIRED)
find_package(LAPACK REQUIRED)

target_sources(${oturb_exe_name} PRIVATE main.cpp)
target_link_libraries(${oturb_exe_name} PRIVATE
    Kokkos::kokkos
    LAPACK::LAPACK
    lapacke
    lapack
    blas
)

target_link_libraries(${oturb_lib_name} PRIVATE
    Kokkos::kokkos
    LAPACK::LAPACK
    lapacke
    lapack
    blas
)

target_include_directories(${oturb_exe_name} PRIVATE ${PROJECT_BINARY_DIR})
target_include_directories(${oturb_exe_name}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
)

# OpenTurbine lib include directories
target_include_directories(${oturb_lib_name}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Get all source files
add_subdirectory(io)
add_subdirectory(rigid_pendulum_poc)
add_subdirectory(utilities)

# Generate OpenTurbine version header
configure_file("${CMAKE_SOURCE_DIR}/cmake/OpenTurbineVersion.H.in"
    "${CMAKE_CURRENT_BINARY_DIR}/OpenTurbineVersion.H" @ONLY
)

target_link_libraries(${oturb_exe_name} PRIVATE ${oturb_lib_name})

# Set -fpic options
set_target_properties(${oturb_lib_name}
    PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

file(GLOB_RECURSE openturbine_header_list ${CMAKE_CURRENT_SOURCE_DIR} "*.H")
add_custom_target(openturbine_headers SOURCES ${openturbine_header_list})

# Add CMake-generated headers to installation
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/OpenTurbineVersion.H
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
