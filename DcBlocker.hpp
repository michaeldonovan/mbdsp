/**
 * DC blocker
 *
 * Based on code by hc.niweulb@lossor.ydna, which was based on code by Julius O.
 * Smith III.
 *
 * From
 * https://www.musicdsp.org/en/latest/Filters/135-dc-filter.html?highlight=dc
 */
#ifndef DCBLOCKER_HPP
#define DCBLOCKER_HPP

#include "AudioProcessor.hpp"
#include "Utils.hpp"

namespace mbdsp
{
template <class T = float>
class DcBlocker : public AudioProcessor<T>
{
public:
    using sample_type = T;

    void Init(sample_type sample_rate, sample_type freq_hz = 20.f)
    {
        fs_ = sample_rate;
        R_ = 1.f - (PI * 2.f * freq_hz / fs_);
    }

    inline sample_type Process(sample_type in) override
    {
        auto out = in - last_in_ + R_ * last_out_;
        last_in_ = in;
        last_out_ = out;
        return out;
    }

private:
    sample_type fs_;
    sample_type R_;
    sample_type last_in_ = 0;
    sample_type last_out_ = 0;
};

}  // namespace mbdsp

#endif  // DCBLOCKER_HPP
