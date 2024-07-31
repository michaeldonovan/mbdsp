#pragma once

#include <cmath>
#include <concepts>
#include <functional>
#include <numeric>
#include <gcem.hpp>
#include "../Concepts.hpp"
#include "../Utils.hpp"

namespace mbdsp
{

namespace concepts
{

template <typename T, typename V>
concept range = requires(T t, V val) {
    { t.min_v } -> std::same_as<V>;
    { t.max_v } -> std::same_as<V>;
};

template <typename T, typename V>
concept processor = requires(T t) {
    { t.Process() } -> std::convertible_to<V>;
};

}  // namespace concepts

namespace Remap
{

template <concepts::numeric V>
struct Linear
{
    using value_type = V;

    constexpr value_type operator()(value_type val, value_type min, value_type max)
    {
        return clamp<V>(min + val * (max - min), min, max);
    }
};

template <concepts::numeric V, V exponent = 2.f>
struct Exponential
{
    using value_type = V;

    constexpr value_type operator()(value_type val, value_type min, value_type max)
    {
        return Linear<V>{}(std::pow(val, exponent), min, max);
    }
};

template <concepts::numeric V>
struct Logarithmic
{
    using value_type = V;

    constexpr value_type operator()(value_type val, value_type min, value_type max)
    {
        V logmin = std::log(std::max(min, V{0.0000001}));
        V logmax = std::log(max);
        auto v = std::exp(Linear<V>{}(val, logmin, logmax));
        return clamp<V>(v, min, max);
    }
};

}  // namespace Remap

template <concepts::numeric V>
struct Control
{
    Control() : input_([]() { return V{}; }) {}
    Control(concepts::invocable<V> auto input) { input_ = std::move(input); }

    Control(concepts::processor<V> auto* input) : Control([input]() { return input->Process(); }) {}

    Control<V>& Init(concepts::invocable<V> auto input)
    {
        input_ = input;
        return *this;
    }

    Control<V>& Init(concepts::processor<V> auto* input)
    {
        input_ = [input]() {
            return input->Process();
        };
        return *this;
    }

    Control<V>& Scale(V coeff)
    {
        functors_.push_back([coeff](V val) { return val * coeff; });
        return *this;
    }

    Control<V>& Exp(V base)
    {
        functors_.push_back([base](V val) { return std::pow(base, val); });
        return *this;
    }

    Control<V>& Pow(V power)
    {
        functors_.push_back([power](V val) { return std::pow(val, power); });
        return *this;
    }

    Control<V>& Offset(V offset)
    {
        functors_.push_back([offset](V val) { return val + offset; });
        return *this;
    }

    Control<V>& Invert()
    {
        functors_.push_back([](V val) { return val * V{-1}; });
        return *this;
    }

    template <class mapping = Remap::Linear<V>>
    Control<V>& Remap(V min, V max)
    {
        functors_.push_back([min, max](V&& val) { return mapping{}(val, min, max); });
        return *this;
    }

    Control<V>& Clamp(V min, V max)
    {
        functors_.push_back([min, max](V&& val) { return clamp(val, min, max); });
        return *this;
    }

    Control<V>& Add(std::convertible_to<Control<V>> auto&& control)
    {
        auto ctrl = Control<V>{std::forward<decltype(control)>(control)};
        functors_.push_back([ctrl = std::move(ctrl)](V&& val) mutable { return val + ctrl(); });
        return *this;
    }

    Control<V>& Multiply(std::convertible_to<Control<V>> auto&& control)
    {
        auto ctrl = Control<V>{std::forward<decltype(control)>(control)};
        functors_.push_back([ctrl = std::move(ctrl)](V&& val) mutable { return val * ctrl(); });
        return *this;
    }

    V operator()()
    {
        return std::accumulate(functors_.begin(), functors_.end(), input_(),
                               [](V val, auto&& func) { return func(val); });
    }

protected:
    std::function<V()> input_;
    std::vector<std::function<V(V)>> functors_;
};

template <concepts::numeric V, V fine_semitones = 12.f>
Control<V> PitchControl(std::convertible_to<Control<V>> auto&& v_oct,
                        std::convertible_to<Control<V>> auto&& coarse, V coarse_min, V coarse_max,
                        std::convertible_to<Control<V>> auto&& fine, V max_voltage = 5.f)
{
    constexpr auto semitone_coeff = gcem::pow(2.f, 1.f / 12.f);

    using Remap::Exponential;

    auto v_oct_ctrl =
        Control<V>{std::forward<decltype(v_oct)>(v_oct)}.Remap(0, max_voltage).Exp(2.f);

    auto coarse_ctrl =
        Control<V>{std::forward<decltype(coarse)>(coarse)}.template Remap<Exponential<V, 2.f>>(
            coarse_min, coarse_max);

    auto fine_ctrl = Control<V>{std::forward<decltype(fine)>(fine)}.Exp(semitone_coeff);

    return coarse_ctrl.Multiply(std::move(v_oct_ctrl)).Multiply(std::move(fine_ctrl));
}

template <concepts::numeric V>
Control<V> PitchControl(std::convertible_to<Control<V>> auto&& v_oct,
                        std::convertible_to<Control<V>> auto&& coarse, V coarse_min, V coarse_max,
                        V max_voltage = 5.f)
{
    using Remap::Exponential;

    auto v_oct_ctrl =
        Control<V>{std::forward<decltype(v_oct)>(v_oct)}.Remap(0, max_voltage).Exp(2.f);

    auto coarse_ctrl =
        Control<V>{std::forward<decltype(coarse)>(coarse)}.template Remap<Exponential<V, 2.f>>(
            coarse_min, coarse_max);

    return coarse_ctrl.Multiply(std::move(v_oct_ctrl));
}

}  // namespace mbdsp