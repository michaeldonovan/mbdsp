#pragma once

#include "../Concepts.hpp"

namespace mbdsp
{

namespace concepts
{

template <typename T, typename V>
concept control_input = requires(T t) {
    { t.Process() } -> std::convertible_to<V>;
};

}  // namespace concepts

template <concepts::numeric V>
struct Control
{
    Control() : func([]() { return V{}; }) {}
    Control(concepts::invocable<float> auto input) { func = std::move(input); }

    Control(concepts::control_input<float> auto input)
        : Control([&input]() { return input.Process(); })
    {
    }

    V operator()() { return func(); }

    std::function<V()> func;
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