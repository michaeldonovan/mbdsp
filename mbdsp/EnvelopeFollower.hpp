/**
 * Based on
 * https://www.musicdsp.org/en/latest/Filters/265-output-limiter-using-envelope-follower-in-c.html
 */
#ifndef EnvFollower_h
#define EnvFollower_h

#include <cmath>
#include <numbers>
#include <vector>
#include "Concepts.hpp"
#include "OnePoleLpf.hpp"
#include "Utils.hpp"

namespace mbdsp
{

std::array<float, 9> COEFFS = {-37.52,    0.3435,    0.008664,   -0.001173, -7.878E-5,
                               -2.901E-6, -5.916E-8, -6.298E-10, -2.739E-12};

template <typename Coeffs>
float polynomial(const Coeffs& coeffs, float x)
{
    float out = 0;
    float factor = 1;
    for(auto& coeff : coeffs)
    {
        out += coeff * factor;
        factor *= x;
    }
    return out;
}

template <concepts::numeric T = float>
class EnvFollower
{
public:
    using sample_type = T;

    enum EnvMode
    {
        PEAK,
        RMS
    };

    virtual void Init(EnvMode detect_mode, sample_type attack_ms, sample_type release_ms,
                      sample_type hold_ms, sample_type sample_rate)
    {
        mode_ = detect_mode;
        rms_lpf_.Init(100.f, sample_rate);
        SetAttack(attack_ms);
        SetRelease(release_ms);
        SetHold(hold_ms);
        env_ = 0;
        timer_ = 0;
        fs_ = sample_rate;
    }

    sample_type Process(sample_type sample)
    {
        sample_type val = fabs(sample);
        if(mode_ == RMS) { val = sqrtf(rms_lpf_.Process(val * val)); }

        if(val > env_)
        {
            env_ = attack_coeff_ * (env_ - val) + val;
            timer_ = 0;
        }
        else if(timer_ < hold_) { timer_++; }
        else { env_ = release_coeff_ * (env_ - val) + val; }

        return env_;
    }

    inline void SetAttack(sample_type attack_ms)
    {
        attack_ms_ = attack_ms;
        attack_coeff_ = std::exp(-1.0 / ms_to_samples(attack_ms, fs_));
    }

    inline void SetRelease(sample_type release_ms)
    {
        release_ms_ = release_ms;
        release_coeff_ = std::exp(-1.0 / ms_to_samples(release_ms, fs_));
    }

    inline void SetRmsSize(sample_type ms) { rms_lpf_.SetTimeConstant(ms); }

    inline void SetHold(sample_type hold_ms) { hold_ = ms_to_samples(hold_ms, fs_); }
    void Setmode(EnvMode mode) { mode_ = mode; }

    inline sample_type GetAttack() const { return attack_ms_; }
    inline sample_type GetRelease() const { return release_ms_; }

    inline sample_type GetHold() const { return samples_to_ms(hold_); }

    inline sample_type Value() const { return env_; }

protected:
    OnePoleLpf<sample_type> rms_lpf_;
    sample_type attack_ms_;
    sample_type release_ms_;
    sample_type attack_coeff_;
    sample_type release_coeff_;
    sample_type env_;
    sample_type fs_;
    sample_type timer_;
    sample_type hold_;
    EnvMode mode_;
};

}  // namespace mbdsp

#endif /* EnvFollower_h */
