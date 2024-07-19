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

}  // namespace concepts
}  // namespace mbdsp
