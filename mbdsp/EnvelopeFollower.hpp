/**
 * Based on
 * https://www.musicdsp.org/en/latest/Filters/265-output-limiter-using-envelope-follower-in-c.html
 */
#ifndef EnvFollower_h
#define EnvFollower_h

#include <cmath>
#include <vector>

#include "AudioProcessor.hpp"
#include "Utils.hpp"

namespace mbdsp
{

template <class T = float>
class EnvFollower : public AudioProcessor<T>
{
public:
    using sample_type = T;

    enum class EnvMode
    {
        PEAK,
        RMS
    };

    virtual void Init(EnvMode detect_mode, float attack_ms, float release_ms, float hold_ms,
                      float sample_rate)
    {
        mode_ = detect_mode;
        fs_ = sample_rate;
        attack_ = mbdsp::powf_approx(0.01, 1.0 / (attack_ms * fs_ * 0.001));
        release_ = mbdsp::powf_approx(0.01, 1.0 / (release_ms * fs_ * 0.001));
        hold_ = hold_ms / 1000. * fs_;
        env_ = 0;
        timer_ = 0;
        rms_window_len_ = sample_rate * 0.2;
        buffer_.resize(rms_window_len_);
        index_ = 0;
    }

    sample_type Process(sample_type sample) override
    {
        float mag;
        if(mode_ == EnvMode::RMS) {}
        else { mag = std::abs(sample); }
        if(mag > env_)
        {
            env_ = attack_ * (env_ - mag) + mag;
            timer_ = 0;
        }
        else if(timer_ < hold_) { timer_++; }
        else { env_ = release_ * (env_ - mag) + mag; }

        return env_;
    }

    inline void SetAttack(float attack_ms) { attack_ = attack_ms; }

    inline void SetRelease(float release_ms) { release_ = release_ms; }

    inline void SetHold(float hold_ms) { hold_ = hold_ms; }

    inline void SetDetectMode(EnvMode mode) { mode_ = mode; }

    inline float GetAttack() { return attack_; }
    inline float GetRelease() { return release_; }
    inline float GetHold() { return hold_; }

protected:
    std::vector<sample_type> buffer_;
    float attack_;
    float release_;
    float env_;
    float fs_;
    float timer_;
    float hold_;
    size_t index_;
    size_t rms_window_len_;
    EnvMode mode_;
};

template <class T = float>
class Compressor : public EnvFollower<T>
{
public:
    using sample_type = T;

    enum class CompMode
    {
        COMP,
        LIMIT
    };

    void Init(float attack_ms, float release_ms, float hold_ms, float ratio, float knee,
              float sample_rate)
    {
        EnvFollower<T>::Init(EnvFollower<T>::EnvMode::PEAK, attack_ms, release_ms, hold_ms,
                             sample_rate);
        comp_mode_ = CompMode::COMP;
        gain_reduction_ = 0;
        knee_ = knee;
        ratio_ = ratio;
        threshold_ = 0.;
        CalcKnee();
        CalcSlope();
    }

    inline void SetAttack(float attack_ms)
    {
        this->attack_ = mbdsp::powf_approx(0.01, 1.0 / (attack_ms * this->fs_ * 0.001));
    }

    inline void SetRelease(float release_ms)
    {
        this->release_ = mbdsp::powf_approx(0.01, 1.0 / (release_ms * this->fs_ * 0.001));
    }

    inline void SetHold(float hold_ms) { this->hold_ = hold_ms / 1000. * this->fs_; }

    inline void SetKnee(float knee)
    {
        knee_ = knee;
        CalcKnee();
        CalcSlope();
    }

    inline void SetRatio(float ratio)
    {
        ratio_ = ratio;
        CalcKnee();
        CalcSlope();
    }

    inline void SetThreshold(float thresholdDB)
    {
        threshold_ = thresholdDB;
        CalcKnee();
        CalcSlope();
    }

    inline void SetMode(CompMode mode)
    {
        comp_mode_ = mode;
        CalcSlope();
    }

    inline float GetThreshold() { return threshold_; }

    inline float GetKnee() { return knee_; }
    inline float GetRatio() { return ratio_; }
    inline float GetGainReductionDb() { return gain_reduction_; }

    inline sample_type Process(sample_type sample) override
    {
        auto e = mbdsp::amp_to_db(EnvFollower<T>::Process(sample));
        if(knee_width_ > 0.f && e > knee_lower_bound_ && e < knee_upper_bound_)
        {
            slope_ *= ((e - knee_lower_bound_) / knee_width_) * 0.5;
            gain_reduction_ = slope_ * (knee_lower_bound_ - e);
        }
        else
        {
            gain_reduction_ = slope_ * (threshold_ - e);
            gain_reduction_ = std::min(0.f, gain_reduction_);
        }

        return sample * mbdsp::db_to_amp(gain_reduction_);
    }

    // Takes in two samples, processes them, and returns gain reduction in dB
    inline sample_type ProcessStereo(sample_type sample1, sample_type sample2)
    {
        auto e = mbdsp::amp_to_db(EnvFollower<T>::Process(std::max(sample1, sample2)));
        CalcSlope();

        if(knee_width_ > 0. && e > knee_lower_bound_ && e < knee_upper_bound_)
        {
            slope_ *= ((e - knee_lower_bound_) / knee_width_) * 0.5;
            gain_reduction_ = slope_ * (knee_lower_bound_ - e);
        }
        else
        {
            gain_reduction_ = slope_ * (threshold_ - e);
            gain_reduction_ = std::min(0.f, gain_reduction_);
        }

        return gain_reduction_;
    }

protected:
    float gain_reduction_;
    float knee_;
    float ratio_;
    float threshold_;
    float knee_width_;
    float knee_lower_bound_;
    float knee_upper_bound_;
    float slope_;
    CompMode comp_mode_;

    inline void CalcKnee()
    {
        knee_width_ = threshold_ * knee_ * -1.;
        knee_lower_bound_ = threshold_ - (knee_width_ / 2.);
        knee_upper_bound_ = threshold_ + (knee_width_ / 2.);
    }

    inline void CalcSlope()
    {
        if(comp_mode_ == CompMode::COMP) { slope_ = 1 - (1 / ratio_); }
        else if(comp_mode_ == CompMode::LIMIT) { slope_ = 1; }
    }
};

}  // namespace mbdsp

#endif /* EnvFollower_h */
