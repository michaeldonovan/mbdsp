cmake_minimum_required(VERSION 3.15)

project(mbdsp LANGUAGES CXX)

add_library(mbdsp INTERFACE)

set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR})

target_include_directories(mbdsp INTERFACE 
                            ${SRC_DIR}
                            ${SRC_DIR}/thirdparty
                            ${SRC_DIR}/thirdparty/hiir-1.4.0
                        )
set_target_properties(mbdsp PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED On
    )

if (BUILD_TESTING AND ${CMAKE_PROJECT_NAME} STREQUAL mbdsp)
    Include(FetchContent)

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