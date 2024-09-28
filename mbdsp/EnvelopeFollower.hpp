/**
 * Based on
 * https://www.musicdsp.org/en/latest/Filters/265-output-limiter-using-envelope-follower-in-c.html
 */
#ifndef EnvFollower_h
#define EnvFollower_h

#include <cmath>
#include <vector>
#include "Concepts.hpp"
#include "Utils.hpp"

namespace mbdsp
{

template <concepts::numeric T = float>
class EnvFollower
{
public:
    using sample_type = T;

    enum class EnvMode
    {
        PEAK,
        RMS
    };

    virtual void Init(EnvMode detect_mode, sample_type attack_ms, sample_type release_ms,
                      sample_type hold_ms, sample_type sample_rate)
    {
        mode_ = detect_mode;
        fs_ = sample_rate;
        attack_ = mbdsp::powf_approx(0.01, 1.0 / (attack_ms * fs_ * 0.001));
        release_ = mbdsp::powf_approx(0.01, 1.0 / (release_ms * fs_ * 0.001));
        hold_ = hold_ms / 1000. * fs_;
        env_ = 0;
        timer_ = 0;
        rms_window_len_ = sample_rate * 0.2;
        // if(detect_mode == RMS) { buffer_.reserve(rms_window_len_); }
        index_ = 0;
    }

    sample_type Process(sample_type sample)
    {
        sample_type mag;
        if(mode_ == EnvMode::RMS) {}
        else { mag = gcem::fabs(sample); }
        if(mag > env_)
        {
            env_ = attack_ * (env_ - mag) + mag;
            timer_ = 0;
        }
        else if(timer_ < hold_) { timer_++; }
        else { env_ = release_ * (env_ - mag) + mag; }

        return env_;
    }

    inline void SetAttack(sample_type attack_ms) { attack_ = attack_ms; }

    inline void SetRelease(sample_type release_ms) { release_ = release_ms; }

    inline void SetHold(sample_type hold_ms) { hold_ = hold_ms; }

    inline void SetDetectMode(EnvMode mode) { mode_ = mode; }

    inline sample_type GetAttack() const { return attack_; }
    inline sample_type GetRelease() const { return release_; }
    inline sample_type GetHold() const { return hold_; }

    inline sample_type Value() const { return env_; }

protected:
    std::vector<sample_type> buffer_;
    sample_type attack_;
    sample_type release_;
    sample_type env_;
    sample_type fs_;
    sample_type timer_;
    sample_type hold_;
    size_t index_;
    size_t rms_window_len_;
    EnvMode mode_;
};

template <concepts::numeric T = float>
class Compressor
{
public:
    using sample_type = T;

    enum class CompMode
    {
        COMP,
        LIMIT
    };

    enum class Topology
    {
        FEEDBACK,
        FEEDFORWARD
    };

    void Init(sample_type attack_ms, sample_type release_ms, sample_type hold_ms, sample_type ratio,
              sample_type knee, sample_type sample_rate, bool makeup = true,
              Topology topology = Topology::FEEDBACK)
    {
        env_follower_.Init(EnvFollower<sample_type>::EnvMode::PEAK, attack_ms, release_ms, hold_ms,
                           sample_rate);
        comp_mode_ = CompMode::COMP;
        gain_reduction_ = 0;
        knee_ = knee;
        ratio_ = ratio;
        threshold_ = 0.;
        makeup_ = makeup;
        gain_ = 0;
        topology_ = topology;
        CalcKnee();
        CalcSlope();
    }

    inline void SetAttack(sample_type attack_ms)
    {
        this->attack_ = mbdsp::powf_approx(0.01, 1.0 / (attack_ms * this->fs_ * 0.001));
    }

    inline void SetRelease(sample_type release_ms)
    {
        this->release_ = mbdsp::powf_approx(0.01, 1.0 / (release_ms * this->fs_ * 0.001));
    }

    inline void SetHold(sample_type hold_ms) { this->hold_ = hold_ms * this->fs_ * 0.001; }

    inline void SetKnee(sample_type knee)
    {
        knee_ = knee;
        CalcKnee();
        CalcSlope();
    }

    inline void SetRatio(sample_type ratio)
    {
        ratio_ = ratio;
        CalcKnee();
        CalcSlope();
        CalcMakeup();
    }

    inline void SetThreshold(sample_type thresholdDB)
    {
        threshold_ = thresholdDB;
        CalcKnee();
        CalcSlope();
        CalcMakeup();
    }

    inline void SetMode(CompMode mode)
    {
        comp_mode_ = mode;
        CalcSlope();
    }

    inline sample_type GetThreshold() { return threshold_; }

    inline sample_type GetKnee() { return knee_; }
    inline sample_type GetRatio() { return ratio_; }
    inline sample_type GetGainReductionDb() { return gain_reduction_; }

    inline sample_type Process(sample_type sample)
    {
        if(topology_ == Topology::FEEDFORWARD) { env_follower_.Process(sample); }

        const auto env = amp_to_db(env_follower_.Value());

        if(knee_width_ > 0.f && env > knee_lower_bound_ && env < knee_upper_bound_)
        {
            slope_ *= ((env - knee_lower_bound_) / knee_width_) * 0.5;
            gain_reduction_ = slope_ * (knee_lower_bound_ - env);
        }
        else
        {
            gain_reduction_ = slope_ * (threshold_ - env);
            gain_reduction_ = gcem::min(0.f, gain_reduction_);
        }

        sample *= mbdsp::db_to_amp(gain_reduction_);

        if(topology_ == Topology::FEEDBACK) { env_follower_.Process(sample); }

        return sample * mbdsp::db_to_amp(gain_);
    }

protected:
    EnvFollower<sample_type> env_follower_;
    sample_type gain_reduction_;
    sample_type knee_;
    sample_type ratio_;
    sample_type threshold_;
    sample_type knee_width_;
    sample_type knee_lower_bound_;
    sample_type knee_upper_bound_;
    sample_type slope_;
    sample_type gain_;
    CompMode comp_mode_;
    Topology topology_;
    bool makeup_;

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

    inline void CalcMakeup()
    {
        if(makeup_) { gain_ = gcem::fabs(threshold_ - threshold_ / ratio_) * 0.1; }
    }
};

}  // namespace mbdsp

#endif /* EnvFollower_h */
