#include <catch2/catch_test_macros.hpp>

#include "mbdsp/Controls/Parameter.hpp"

using namespace mbdsp;

TEST_CASE("Single-input param returns correct value", "[Parameter1Input]")
{
    Parameter<float, Remap::Linear<float, 0.f, 10.f>, 1> param;
    float setVal = 1;
    auto controlFn = [&setVal]() {
        return setVal;
    };

    param.Init(controlFn);

    for(setVal = 0; setVal < 10; ++setVal)
    {
        REQUIRE(param() == setVal * decltype(param)::mapping_type::max_v);
    }
}

TEST_CASE("Two-input param returns correct value", "[Parameter2Input]")
{
    Parameter<float, Remap::Linear<float, 0.f, 10.f>, 2> param;
    float setVal = 1;
    auto controlFn = [&setVal]() {
        return setVal;
    };
    auto controlFn2 = []() {
        return .5;
    };

    param.Init(controlFn, controlFn2);

    for(setVal = 0; setVal < 1 - controlFn2(); ++setVal)
    {
        REQUIRE(param() == (setVal + controlFn2()) * decltype(param)::mapping_type::max_v);
    }
}