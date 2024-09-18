#pragma once
#include <concepts>

namespace mbdsp
{
namespace concepts
{
template <typename T>
concept numeric = std::integral<T> || std::floating_point<T>;

template <typename T, typename R, typename... Args>
concept invocable = std::is_invocable_r_v<R, T, Args...>;

template <typename T, typename V>
concept functor = std::is_invocable_r_v<V, T, V>;

template <typename T, typename V>
concept range = requires(T t, V val) {
    { t.min_v } -> std::same_as<V>;
    { t.max_v } -> std::same_as<V>;
};

template <typename T, typename V>
concept processor = requires(T t) {
    { t.Process() } -> std::convertible_to<V>;
};

template <typename T, typename V>
concept value_provider = requires(T t) {
    { t.Value() } -> std::convertible_to<V>;
};

}  // namespace concepts
}  // namespace mbdsp
