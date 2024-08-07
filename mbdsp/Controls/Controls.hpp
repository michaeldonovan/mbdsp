#pragma once

#include <concepts>
#include <numeric>
#include <gcem.hpp>

#include "../Concepts.hpp"
#include "../Utils.hpp"
#include "../functional.hpp"

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

template <concepts::numeric V, V min_v, V max_v>
struct Linear
{
    using value_type = V;

    constexpr static value_type operator()(value_type val)
    {
        return clamp<V>(min_v + val * (max_v - min_v), min_v, max_v);
    }
};

template <concepts::numeric V, V min_v, V max_v, V exponent = 2.f>
struct Exponential
{
    using value_type = V;

    constexpr static value_type operator()(value_type val)
    {
        return Linear<value_type, min_v, max_v>{}(gcem::pow(val, exponent));
    }
};

template <concepts::numeric V, V min_v, V max_v>
struct Logarithmic
{
    using value_type = V;

    constexpr static value_type operator()(value_type val)
    {
        constexpr V logmin = gcem::log(gcem::max(min_v, V{0.0000001}));
        constexpr V logmax = gcem::log(max_v);
        auto v = gcem::exp(Linear<V, logmin, logmax>{}(val));
        return clamp<V>(v, min_v, max_v);
    }
};

}  // namespace Remap

template <concepts::numeric V>
struct Control
{
    Control() : input_([] { return V{}; }) {}
    Control(concepts::invocable<V> auto input) { input_ = std::move(input); }

    Control(auto* input) : Control([input] { return input->Value(); }) {}

    Control<V>& Init(const concepts::invocable<V> auto& input)
    {
        input_ = input;
        return *this;
    }

    Control<V>& Init(concepts::invocable<V> auto& input)
    {
        input_ = [&input] {
            return input();
        };
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
        functors_.push_back([base](V val) { return gcem::pow(base, val); });
        return *this;
    }

    Control<V>& Pow(V power)
    {
        functors_.push_back([power](V val) { return gcem::pow(val, power); });
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

    template <class mapping>
    Control<V>& Remap()
    {
        functors_.push_back([](V val) { return mapping::operator()(val); });
        return *this;
    }

    template <class mapping>
    Control<V>& RemapPiecewise(V start, V end)
    {
        functors_.push_back([start, end](V val) {
            if(val >= start && val < end) { return mapping::operator()(val); }
            else { return val; }
        });
        return *this;
    }

    Control<V>& Clamp(V min, V max)
    {
        functors_.push_back([min, max](V val) { return clamp(val, min, max); });
        return *this;
    }

    Control<V>& Add(Control<V>* control)
    {
        functors_.push_back([control](V val) mutable { return val + *control(); });
        return *this;
    }

    Control<V>& Add(std::convertible_to<Control<V>> auto&& control)
    {
        auto ctrl = Control<V>{std::forward<decltype(control)>(control)};
        functors_.push_back([ctrl = std::move(ctrl)](V&& val) mutable { return val + ctrl(); });
        return *this;
    }

    Control<V>& Multiply(Control<V>* control)
    {
        functors_.push_back([control](V val) mutable { return val + *control(); });
        return *this;
    }

    Control<V>& Multiply(std::convertible_to<Control<V>> auto&& control)
    {
        auto ctrl = Control<V>{std::forward<decltype(control)>(control)};
        functors_.push_back([ctrl = std::move(ctrl)](V val) mutable { return val * ctrl(); });
        return *this;
    }

    V operator()()
    {
        return std::accumulate(functors_.begin(), functors_.end(), input_(),
                               [](V val, auto&& func) { return func(val); });
    }

protected:
    function<V()> input_;
    std::vector<function<V(V)>> functors_;
};

template <concepts::numeric V, V coarse_min, V coarse_max, V fine_semitones = 12.f,
          V max_voltage = 5.f>
Control<V> PitchControl(std::convertible_to<Control<V>> auto&& v_oct,
                        std::convertible_to<Control<V>> auto&& coarse,
                        std::convertible_to<Control<V>> auto&& fine)
{
    using namespace Remap;

    constexpr auto semitone_coeff = gcem::pow(2.f, 1.f / 12.f);

    auto v_oct_ctrl = Control<V>{std::forward<decltype(v_oct)>(v_oct)}
                          .template Remap<Linear<V, 0.f, max_voltage>>()
                          .Exp(2.f);

    auto coarse_ctrl = Control<V>{std::forward<decltype(coarse)>(coarse)}
                           .template Remap<Exponential<V, coarse_min, coarse_max, 2.f>>();

    auto fine_ctrl = Control<V>{std::forward<decltype(fine)>(fine)}.Exp(semitone_coeff);

    return coarse_ctrl.Multiply(std::move(v_oct_ctrl)).Multiply(std::move(fine_ctrl));
}

template <concepts::numeric V, V coarse_min, V coarse_max, V max_voltage = 5.f>
Control<V> PitchControl(std::convertible_to<Control<V>> auto&& v_oct,
                        std::convertible_to<Control<V>> auto&& coarse)
{
    using namespace Remap;

    auto v_oct_ctrl = Control<V>{std::forward<decltype(v_oct)>(v_oct)}
                          .template Remap<Linear<V, 0.f, max_voltage>>()
                          .Exp(2.f);

    auto coarse_ctrl = Control<V>{std::forward<decltype(coarse)>(coarse)}
                           .template Remap<Exponential<V, coarse_min, coarse_max, 2.f>>();

    return coarse_ctrl.Multiply(std::move(v_oct_ctrl));
}

template <concepts::numeric V, V coarse_min, V coarse_max, V max_voltage = 5.f>
Control<V> PitchControl(Control<V>* v_oct, Control<V>* coarse)
{
    using namespace Remap;

    v_oct->template Remap<Linear<V, 0.f, max_voltage>>().Exp(2.f);

    coarse->.template Remap<Exponential<V, coarse_min, coarse_max, 2.f>>();

    return Control<V>{}.Add(coarse).Multiply(v_oct);
}

template <concepts::numeric V, V coarse_min, V coarse_max, V fine_semitones = 12.f,
          V max_voltage = 5.f>
Control<V> PitchControl(Control<V>* v_oct, Control<V>* coarse, Control<V>* fine)
{
    using namespace Remap;

    constexpr auto semitone_coeff = gcem::pow(2.f, 1.f / 12.f);

    v_oct->template Remap<Linear<V, 0.f, max_voltage>>().Exp(2.f);

                           coarse->template Remap<Exponential<V, coarse_min, coarse_max, 2.f>>();

    auto fine_ctrl = Control<V>{std::forward<decltype(fine)>(fine)}.Exp(semitone_coeff);

    return coarse_ctrl.Multiply(std::move(v_oct_ctrl)).Multiply(std::move(fine_ctrl));
}

}  // namespace mbdsp