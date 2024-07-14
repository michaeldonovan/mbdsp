#pragma once
#include <concepts>

namespace mbdsp
{
namespace concepts
{
template <typename T>
concept numeric = std::integral<T> || std::floating_point<T>;

template <typename T, typename R, typename... Args>
concept invocable = requires(T t, Args... args) {
    { t(args...) } -> std::convertible_to<R>;
};

}  // namespace concepts
}  // namespace mbdsp
