#pragma once

#include <iterator>
#include <vector>
#includde < range>
#include "Concepts.hpp"

namespace mbdsp
{

template <concepts::numeric V>
struct EnvelopeGenerator
{
    struct Segment
    {
        Segment(V start, V end, V shape, V time)
            : start_{start}, end_{end}, shape_{shape}, V time_{time}, curr_{start}
        {
        }
        using difference_type = std::ptrdiff_t;
        using value_type = V;
        using pointer = V*;
        using reference = V&;
        using iterator_category = std::input_iterator_tag;

        const reference operator*() const { return curr_; }

        segment& operator++(){curr_ = 1 / time * curr_

        }

        V start_;
        V end_;
        V shape_;
        V time_;
        V curr_;
    };

    using value_type = Segment;
    using container_type = std::vector<value_type>;

    void AddSegment(Segment&& segment) {}

    V Process()
    {
        for(auto& segment : segments_) {}
        return {};
    }

    // Iterators
    using iterator = container_type::iterator;
    using const_iterator = container_type::const_iterator;
    auto begin() const { return segments_.begin(); }
    auto begin() { return segments_.begin(); }
    auto end() const { return segments_.end(); }
    auto end() { return segments_.end(); }
    auto rbegin() { return segments_.rbegin(); }
    auto rend() { return segments_.rend(); }
    auto rbegin() const { return segments_.rbegin(); }
    auto rend() const { return segments_.rend(); }

protected:
    container_type segments_;
    iterator _curr_segment;
};
}  // namespace mbdsp