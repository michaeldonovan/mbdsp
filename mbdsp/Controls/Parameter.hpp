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

namespace Remap
{

template <typename T, typename V = float>
concept Mapping = requires(T t, V val) {
    { t(val) } -> std::same_as<V>;
};

template <typename V, V min, V max>
struct MappingBase
{
    using value_type = V;
    static constexpr value_type min_v = min;
    static constexpr value_type max_v = max;
};

template <typename V, V min, V max>
struct Linear : public MappingBase<V, min, max>
{
    using value_type = V;

    static constexpr value_type operator()(value_type val) { return min + val * (max - min); }
};

template <typename V, V min, V max, V exp = 2.f>
struct Exponential : public MappingBase<V, min, max>
{
    using value_type = V;

    static constexpr value_type operator()(value_type val)
    {
        return Linear<V, min, max>()(std::pow(val, exp));
    }
};

template <concepts::numeric V, V min, V max>
struct Logarithmic : public MappingBase<V, min, max>
{
    using value_type = V;

    static constexpr V operator()(V val)
    {
        constexpr V logmin = std::log(std::max(min, V{0.0000001}));
        constexpr V logmax = std::log(max);
        return std::exp(Linear<V, logmin, logmax>()(val));
    }
};

template <Mapping M>
struct Clamp
{
    using value_type = M::value_type;

    static constexpr value_type operator()(value_type val)
    {
        return clamp<value_type>(val, M::min_v, M::max_v);
    }
};

}  // namespace Remap

template <concepts::numeric V, Remap::Mapping<V> M, size_t NumInputs>
class Parameter
{
public:
    using value_type = V;
    using mapping_type = M;

    inline void Init(std::convertible_to<Control<V>> auto&&... controls)
    {
        static_assert(sizeof...(controls) == NumInputs);
        controls_ = decltype(controls_){std::forward<decltype(controls)>(controls)...};
    }

    inline value_type operator()()
    {
        value_type sum = 0;
        for(auto& control : controls_) { sum += control(); }

        return mapping_type{}(clamp(sum, -1.f, 1.f));
    }

    std::array<Control<V>, NumInputs> controls_;
};

}  // namespace mbdsp
