#pragma once

#include <array>
#include <cmath>
#include <functional>
#include <memory>
#include "../Concepts.hpp"
#include "../Utils.hpp"
#include "Controls.hpp"

namespace mbdsp
{

template <concepts::numeric V, V min_v, V max_v>
struct Clamp
{
    V operator()(V val) { return clamp(val, min_v, max_v); }
};

template <concepts::numeric V>
struct Identity
{
    V operator()(V val) { return val; }
};

template <concepts::numeric V, size_t NumInputs = 1, concepts::functor<V> Modifier = Identity<V>>
class Parameter
{
public:
    using value_type = V;

    inline void Init(std::convertible_to<Control<V>> auto&&... controls)
    {
        static_assert(sizeof...(controls) == NumInputs);
        controls_ = decltype(controls_){std::forward<decltype(controls)>(controls)...};
    }

    inline value_type operator()()
    {
        value_type sum = 0;
        size_t i = 0;
        for(auto& control : controls_)
        {
            vals_[i] = control();
            sum += vals_[i];
            ++i;
        }

        return Modifier{}(sum);
    }

    std::array<Control<V>, NumInputs> controls_;
    std::array<V, NumInputs> vals_;
    V val_;
};

}  // namespace mbdsp

