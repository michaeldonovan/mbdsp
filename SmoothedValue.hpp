/**
 * Based on CParamSmooth from the musicdsp.org mailing list
 * https://www.musicdsp.org/en/latest/Filters/257-1-pole-lpf-for-smooth-parameter-changes.html?highlight=smooth
 */

#include <cmath>

#include "DspUtils.hpp"
namespace mbdsp
{

template <class T>
class SmoothedValue
{
public:
    using value_type = T;

    void Init(value_type sample_rate, value_type smooth_time_ms = 100, value_type initial_val = 0)
    {
        a_ = std::exp(-mbdsp::twopi() / (smooth_time_ms * 0.001 * sample_rate));
        b_ = 1.0 - a_;
        curr_ = initial_val;
        SetTarget(initial_val);
    }

    inline value_type Process()
    {
        if(done_smoothing_) { return target_; }

        if(curr_ == target_)
        {
            done_smoothing_ = true;
            return target_;
        }

        curr_ = c_ + (curr_ * a_);
        return curr_;
    }

    inline void SetTarget(float target)
    {
        target_ = target;
        c_ = target_ * b_;
        done_smoothing_ = false;
    }

    inline float GetCurrent() { return curr_; }

private:
    value_type a_;
    value_type b_;
    value_type c_;
    value_type target_;
    value_type curr_;
    bool done_smoothing_;
};

}  // namespace mbdsp