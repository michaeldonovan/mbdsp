#pragma once

#include <experimental/ranges/iterator>
#include <iterators>
#include <ranges>

namespace mbdsp
{
namespace views
{

template <bool>
struct const_if
{
    template <typename T>
    using value_type = T const;
};

template <>
struct const_if<false, T>
{
    template <typename T>
    using value_type = T;
};

template <typename T>
using const_if_t = typename const_if<std::is_const_v<T>>::template value_type<T>;

template <std::ranges::range T>
struct cycle : public std::ranges::view_interface<cycle<T>>
{
public:
    explicit cycle(T rng) : range_{std::move(rng)} {}

    auto begin() -> iterator<false> { return {this}; }
    auto begin() const -> iterator<true> { return {this}; }
    auto end() { return std::default_sentinel{}; }
    auto end() const { return std::default_sentinel{}; }

private:
    template <bool IsConst>
    struct iterator
    {
        using view = const_if_t<IsConst, cycle<T>>;
        using T_iterator = std::ranges::iterator_t<view>;

        iterator(view* v) : view_{v}, it_{std::begin(view_->range_)} {}

        using difference_type = std::iterator_traits<T_iterator>::difference_type;
        using value_type = std::iterator_traits<T_iterator>::value_type;
        using pointer = std::iterator_traits<T_iterator>::pointer;
        using reference = std::iterator_traits<T_iterator>::reference;
        using iterator_category = std::iterator_traits<T_iterator>::iterator_category;

        const reference operator*() const { return *it_; }

        reference operator*()
            requires(!IsConst)
        {
            return *it_;
        }

        iterator& operator++()
            requires std::ranges::forward_iterator<T_iterator>
        {
            if(++it_ == range_.end()) { it_ = std::begin(range_); }
            return *this;
        }

        iterator& operator--()
            requires std::ranges::bidirectional_iterator<T_iterator>
        {
            if(it_ == view_->range_.begin()) { it_ = --view_->range_.end(); }
            else { --it_; }
            return *this;
        }

        view* view_;
        T_iterator it_;
    };
    static_assert(std::experimental::ranges::Iterator<iterator>);

    T range_;
};
static_assert(std::ranges::range<cycle<std::views::iota>>);
static_assert(std::ranges::forward_range<cycle<std::views::iota>>);
static_assert(std::ranges::view<cycle<std::views::iota>>);

}  // namespace views

}  // namespace mbdsp