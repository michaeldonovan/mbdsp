cmake_minimum_required(VERSION 3.15)

project(mbdsp LANGUAGES CXX)

set(mbdsp_VERSION 0.1.0)

add_library(mbdsp INTERFACE)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mbdsp)

target_include_directories(mbdsp INTERFACE 
                            $<BUILD_INTERFACE:${SRC_DIR}>
                            $<BUILD_INTERFACE:${SRC_DIR}/thirdparty>
                            $<BUILD_INTERFACE:${SRC_DIR}/thirdparty/hiir-1.4.0>
                            $<INSTALL_INTERFACE:include>
                        )

target_compile_features(mbdsp INTERFACE cxx_std_20)

# Install headers
install(DIRECTORY ${SRC_DIR}
        DESTINATION include
)

install(TARGETS mbdsp EXPORT mbdspTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Include the helper CMake script that generates the *Config.cmake file.
include(CMakePackageConfigHelpers)

# Create the mbdspConfigVersion.cmake file
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/mbdspConfigVersion.cmake
    VERSION ${mbdsp_VERSION}
    COMPATIBILITY SameMajorVersion
)

export(EXPORT mbdspTargets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/mbdsp/mbdspTargets.cmake"
  NAMESPACE mbdsp::
)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/mbdspConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/mbdspConfig.cmake
    INSTALL_DESTINATION cmake
)

set(ConfigPackageLocation lib/cmake/mbdsp)

install(EXPORT mbdspTargets
    FILE mbdspTargets.cmake
    NAMESPACE mbdsp::
    DESTINATION ${ConfigPackageLocation}
)

# Install the configuration and version files.
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mbdspConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/mbdspConfigVersion.cmake
        DESTINATION ${ConfigPackageLocation}
)

if (BUILD_TESTING AND ${CMAKE_PROJECT_NAME} STREQUAL mbdsp)
    include(FetchContent)

    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.2.0 
    )


    FetchContent_Declare(
        DaisySP
        GIT_REPOSITORY https://github.com/electro-smith/DaisySP
    )

    FetchContent_MakeAvailable(Catch2 DaisySP)
    target_compile_definitions(DaisySP PUBLIC -D_USE_MATH_DEFINES=1)

    add_executable(mbdsp_tests tests/tests.cpp)

    target_link_libraries(mbdsp_tests PRIVATE Catch2::Catch2WithMain)
    target_link_libraries(mbdsp_tests PRIVATE mbdsp DaisySP)


    include(CTest)
    include(Catch)

    catch_discover_tests(mbdsp_tests)
endif()