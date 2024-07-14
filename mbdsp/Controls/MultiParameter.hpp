#pragma once

#include <concepts>
#include <type_traits>
#include <utility>
#include <vector>
#include <daisy.h>

#include "../Concepts.hpp"

namespace mbdsp
{

namespace daisy
{

template <numeric T = float>
class MultiParameter
{
public:
    using value_type = T;

    using Curve = ::daisy::Parameter::Curve;

    template <std::invocable Input>
    void Init(float min, float max, Curve curve, Input... inputs)
    {
        min_ = min;
        max_ = max;
        curve_ = curve;
        (inputs_.emplace_back(std::forward<Input>(inputs)), ...);
    }

    // float Process()
    // {
    //     float sum = 0;
    //     for(auto& [param, mix] : params_) { sum += param.Process() * mix; }
    //     return sum;
    // }

    // float Value()
    // {
    //     float sum = 0;
    //     for(auto& [param, mix] : params_) { sum += param.Value() * mix; }
    //     return sum;
    // }

protected:
    std::vector<std::function<value_type()>> inputs_;
    value_type min_;
    value_type max_;
    Curve curve_;
};
}  // namespace daisy
}  // namespace mbdsp