#ifndef DELAYLINE_HPP
#define DELAYLINE_HPP

#include <vector>

#include "AudioProcessor.hpp"
#include "Utils.hpp"

namespace mbdsp
{

template <class T = float>
class DelayLine : public AudioProcessor<T>
{
public:
    using sample_type = T;

    inline void Init(size_t max_delay) { buffer_.resize(max_delay, 0); }

    inline void Reset()
    {
        for(size_t i = 0; i < buffer_.size(); ++i) { buffer_[i] = 0; }
    }

    inline sample_type Process(sample_type in) override
    {
        auto out = Read(delay_);
        Write(in);
        return out;
    }

    /**
     * @brief Read at a given delay with hermite interpolation
     *
     * @param delay: Delay in samples
     */
    inline sample_type Read(float delay)
    {
        // Hermite interpolation based on code by Laurent de Soras
        // from https://www.musicdsp.org/en/latest/Other/93-hermite-interpollation.html

        const auto buff_size = buffer_.size();
        delay = clamp<float>(delay, 0.f, buff_size);
        const size_t delay_int = delay;
        const float delay_frac = delay - delay_int;

        const size_t x0_idx = write_ptr_ - delay_int + buff_size;
        const auto xm1 = buffer_[(x0_idx + 1) % buff_size];
        const auto x0 = buffer_[x0_idx % buff_size];
        const auto x1 = buffer_[(x0_idx - 1) % buff_size];
        const auto x2 = buffer_[(x0_idx - 2) % buff_size];

        const auto c = (x1 - xm1) * .5f;
        const auto v = x0 - x1;
        const auto w = c + v;
        const auto a = w + v + (x2 - x0) * .5f;
        const auto b_neg = w + a;

        return (((a * delay_frac) - b_neg) * delay_frac + c) * delay_frac + x0;
    }

    inline void Write(sample_type in)
    {
        buffer_[write_ptr_] = in;
        write_ptr_ = (write_ptr_ + 1) % buffer_.size();
    }

    inline void SetDelaySamples(float delay_samples)
    {
        delay_ = clamp(delay_samples, 0, MaxDelay());
    }

    inline float MaxDelay() const { return buffer_.size(); }

private:
    std::vector<sample_type> buffer_;
    float delay_ = 0.f;
    size_t write_ptr_ = 0;
};

}  // namespace mbdsp

#endif  // DELAYLINE_HPP
