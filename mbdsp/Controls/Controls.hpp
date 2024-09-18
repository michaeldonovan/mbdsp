#pragma once

#include <concepts>
#include <numeric>
#include <gcem.hpp>

#include "../Concepts.hpp"
#include "../Utils.hpp"
#include "../functional.hpp"
#include "MakeInvocable.hpp"

namespace mbdsp
{

namespace Remap
{

template <concepts::numeric V, V min_v, V max_v>
struct Linear
{
    using value_type = V;

    static constexpr value_type operator()(value_type val)
    {
        return clamp<V>(min_v + val * (max_v - min_v), min_v, max_v);
    }
};

template <concepts::numeric V, V min_v, V max_v, V exponent = 2.f>
struct Exponential
{
    using value_type = V;

    static constexpr value_type operator()(value_type val)
    {
        return Linear<value_type, min_v, max_v>{}(gcem::pow(val, exponent));
    }
};

template <concepts::numeric V, V min_v, V max_v>
struct Logarithmic
{
    using value_type = V;

    static constexpr value_type operator()(value_type val)
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
    Control(V val) : input_([val] { return val; }) {}
    Control() : Control{V{}} {}

    Control<V>& Init(auto&& input)
        requires concepts::invocable<decltype(input), V>
    {
        // if constexpr(std::convertible_to<decltype(input), decltype(input_)>)
        // {
        //     input_ = std::forward<decltype(input)>(input);
        // }
        // else
        {
            input_ = [in = std::forward<decltype(input)>(input)] {
                return in();
            };
        }

        return *this;
    }

    Control<V>& Init(concepts::value_provider<V> auto* input)
    {
        input_ = [input] {
            return input->Value();
        };
        return *this;
    }

    Control<V>& SetDirtySignal(auto&& fn)
    {
        dirty_signal_ = std::forward<decltype(fn)>(fn);
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
        functors_.push_back([min, max](V val) { return clamp<V>(val, min, max); });
        return *this;
    }

    Control<V>& Apply(concepts::invocable<V> auto&& functor)
    {
        functors_.push_back(std::forward<decltype(functor)>(functor));
        return *this;
    }

    Control<V>& Add(concepts::value_provider<V> auto* input)
    {
        functors_.push_back([input](V val) mutable { return val + input->Value(); });
        return *this;
    }

    Control<V>& Add(concepts::invocable<V> auto&& input)
    {
        functors_.push_back(
            [in = std::forward<decltype(input)>(input)](V val) mutable { return val + in(); });
        return *this;
    }

    Control<V>& Multiply(concepts::value_provider<V> auto* input)
    {
        functors_.push_back([input](V val) mutable { return val * input->Value(); });
        return *this;
    }

    Control<V>& Multiply(concepts::invocable<V> auto&& input)
    {
        functors_.push_back(
            [in = std::forward<decltype(input)>(input)](V val) mutable { return val * in(); });
        return *this;
    }

    V operator()()
    {
        auto newVal = std::accumulate(functors_.begin(), functors_.end(), input_(),
                                      [](V val, auto&& func) { return func(val); });
        if(newVal != value_)
        {
            value_ = newVal;
            dirty_signal_();
        }

        return value_;
    }

protected:
    V value_;
    function<V()> input_;
    function<void()> dirty_signal_ = [] {
    };
    std::vector<function<V(V)>> functors_;
};

template <concepts::numeric V>
struct PitchControl
{
    template <V coarse_min, V coarse_max, V max_voltage = 5.f>
    static PitchControl<V> Build(concepts::invocable<V> auto&& v_oct,
                                 concepts::invocable<V> auto&& coarse)
    {
        using namespace mbdsp::Remap;
        PitchControl<V> ctrl;

        ctrl.v_oct_.Init(std::forward<decltype(v_oct)>(v_oct))
            .template Remap<Linear<V, 0.f, max_voltage>>()
            .Exp(2.f);

        ctrl.coarse_.Init(std::forward<decltype(coarse)>(coarse))
            .template Remap<Exponential<V, coarse_min, coarse_max, 2.f>>();
        ctrl.fine_ = Control<V>{1.f};

        return ctrl;
    }

    template <concepts::invocable<V> input, V coarse_min, V coarse_max, V fine_semitones = 12.f,
              V max_voltage = 5.f>
    static PitchControl<V> Build(concepts::invocable<V> auto&& v_oct,
                                 concepts::invocable<V> auto&& coarse,
                                 concepts::invocable<V> auto&& fine)
    {
        auto ctrl = Build<input, coarse_min, coarse_max, max_voltage>(v_oct, coarse);
        ctrl.fine_.Init(std::forward<decltype(fine)>(fine));
        ctrl.fine_.Exp(semitone_coeff(fine_semitones));

        return ctrl;
    }

    PitchControl<V>& SetDirtySignal(auto&& fn)
    {
        v_oct_.SetDirtySignal(std::cref(fn));
        coarse_.SetDirtySignal(std::cref(fn));
        fine_.SetDirtySignal(std::cref(fn));
        return *this;
    }

    V operator()() { return coarse_() * v_oct_() * fine_(); }

    Control<V> v_oct_;
    Control<V> coarse_;
    Control<V> fine_;
};

}  // namespace mbdsp