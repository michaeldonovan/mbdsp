#ifndef TAPTEMPO_HPP
#define TAPTEMPO_HPP

#include <array>
#include <type_traits>

namespace mbdsp
{
template <class T>
class TapTempo
{
public:
    using time_type = T;
    static_assert(std::is_arithmetic<time_type>::value, "T must be an arithmetic type");

    /**
     * @param max_duration: The maximum time between taps that will be
     * considered
     */
    void Init(time_type max_duration)
    {
        max_dur_ = max_duration;
        Reset();
    }

    time_type Tap(time_type now)
    {
        if(last_ == 0)
        {
            last_ = now;
            return 0;
        }

        auto duration = now - last_;
        last_ = now;

        // if this is the first tap in a while, we'll start our calculations
        // over
        if(duration > max_dur_)
        {
            durations_ = {};
            write_ptr_ = 0;
            return 0;
        }
        else
        {
            write(duration);
            return GetBeatLength();
        }
    }

    time_type GetBeatLength() const
    {
        // calculate mean of taps
        time_type accum = 0;
        time_type num_durs = 0;
        for(const auto& dur : durations_)
        {
            if(dur <= 0) { continue; }

            accum += dur;
            num_durs++;
        }
        if(num_durs) { return accum / num_durs; }
        return 0;
    }

    void Reset()
    {
        durations_ = {};
        write_ptr_ = 0;
        last_ = 0;
    }

private:
    inline void write(time_type duration)
    {
        durations_[write_ptr_] = duration;
        write_ptr_ = (write_ptr_ + 1) % durations_.size();
    }

    time_type last_ = 0;
    std::array<time_type, 4> durations_;
    size_t write_ptr_ = 0;
    time_type max_dur_;
};
}  // namespace mbdsp

#endif  // TAPTEMPO_HPP
