#pragma once

#include <algorithm>
#include "../Concepts.hpp"

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
        return clamp(min + val * (max - min), min, max);
    }
};

template <concepts::numeric V>
struct Exponential
{
    using value_type = V;

    constexpr value_type operator()(value_type val, value_type min, value_type max,
                                    value_type exponent = 2)
    {
        return Linear<V>{}(std::pow(val, exp), min, max);
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
        return clamp(v, min, max);
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
        functors_.push_back([ctrl = std::forward<decltype(control)>(control)](V&& val) {
            return val + Control<V>{ctrl}();
        });
        return *this;
    }

    V operator()()
    {
        // return std::accumulate(functors_.begin(), functors_.end(), input_(),
        //                        [](V val, auto&& func) { return func(val); });
        return std::ranges::fold_left(functors_, input_(),
                                      [](V val, auto&& func) { return func(val); });
    }

protected:
    std::function<V()> input_;
    std::vector<std::function<V(V)>> functors_;
};

template <concepts::numeric V, concepts::invocable<V> C>
Control<V> Scale(C&& ctrl, V coeff)
{
    return [coeff, control = std::move(ctrl)]() {
        return control() * coeff;
    };
}

template <concepts::numeric V, concepts::invocable<V> C>
Control<V> ScaleDb(C&& ctrl, V db)
{
    return [db, control = std::move(ctrl)]() {
        return control() * db_to_amp(db);
    };
}

template <concepts::numeric V, concepts::invocable<V> C>
Control<V> Invert(C&& ctrl)
{
    return [control = std::move(ctrl)]() {
        return control() * -1;
    };
}

template <concepts::numeric V, concepts::invocable<V> C>
Control<V> Offset(C&& ctrl, V offset)
{
    return [offset, control = std::move(ctrl)]() {
        return control() + offset;
    };
}

// template <concepts::numeric V>
// struct ControlWrapper : public Control
// {
//     ControlWrapper(Control&& control) : ctrl_(std::move(control)) {}

//     V operator()() override { return ctrl_(); }

//     Control ctrl_;
// };

// template <concepts::numeric V, V coeff>
// struct Scale : public ControlWrapper<V>
// {
//     Scale(Control&& control) : ctrl_(std::move(control)) {}

//     V operator()() override { return coeff * this->ctrl_(); }
// };

// template <concepts::numeric V, V db>
// struct ScaleDb : public ControlWrapper<V>
// {
//     ScaleDb(Control&& control) : ctrl_(std::move(control)) {}
//     V operator()() override { return db_to_amp(db) * this->ctrl_(); }
// };

// template <concepts::numeric V>
// struct Invert : public ControlWrapper<V>
// {
//     Invert(Control&& control) : ctrl_(std::move(control)) {}
//     V operator()() override { return -1 * this->ctrl_(); }
// };

}  // namespace mbdsp