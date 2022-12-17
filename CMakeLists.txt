cmake_minimum_required(VERSION 3.15)

project(mbdsp LANGUAGES CXX)

add_library(mbdsp INTERFACE)

target_include_directories(mbdsp INTERFACE .)
target_include_directories(mbdsp INTERFACE thirdparty)
target_include_directories(mbdsp INTERFACE thirdparty/hiir-1.4.0)

if (BUILD_TESTING AND ${CMAKE_PROJECT_NAME} STREQUAL mbdsp)
    Include(FetchContent)

    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.2.0 
    )

    FetchContent_MakeAvailable(Catch2)

    add_executable(mbdsp_tests tests/tests.cpp)

    set_property(TARGET mbdsp_tests PROPERTY CXX_STANDARD 20)
    set_property(TARGET mbdsp_tests PROPERTY CXX_STANDARD_REQUIRED On)
    set_property(TARGET mbdsp_tests PROPERTY CXX_EXTENSIONS Off)


    target_link_libraries(mbdsp_tests PRIVATE Catch2::Catch2WithMain)
    target_link_libraries(mbdsp_tests PRIVATE mbdsp)

    include(CTest)
    include(Catch)

    catch_discover_tests(mbdsp_tests)
endif()