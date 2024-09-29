#pragma once
#include "Concepts.hpp"
#include "EnvelopeFollower.hpp"
#include "Utils.hpp"

namespace mbdsp
{

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

    void Init(EnvFollower<sample_type>::EnvMode detector_mode, sample_type attack_ms,
              sample_type release_ms, sample_type hold_ms, sample_type ratio, sample_type knee,
              sample_type sample_rate, bool makeup = true,
              Topology topology = Topology::FEEDFORWARD)
    {
        env_follower_.Init(detector_mode, attack_ms, release_ms, hold_ms, sample_rate);
        env_follower_.SetRmsSize(200.f);
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

    inline void SetAttack(sample_type attack_ms) { env_follower_.SetAttack(attack_ms); }

    inline void SetRelease(sample_type release_ms) { env_follower_.SetRelease(release_ms); }

    inline void SetHold(sample_type hold_ms) { env_follower_.SetHold(hold_ms); }

    inline void SetParams(sample_type threshold_db, sample_type attack_ms, sample_type release_ms,
                          sample_type ratio)
    {
        threshold_ = threshold_db;
        ratio_ = ratio;
        env_follower_.SetAttack(attack_ms);
        env_follower_.SetRelease(release_ms);
        CalcKnee();
        CalcSlope();
        CalcMakeup();
    }

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

    void SetDetectorMode(EnvFollower<sample_type>::EnvMode mode) { env_follower_.SetMode(mode); }
    void SetRmsSize(sample_type ms) { env_follower_.SetRmsSize(ms); }

    inline sample_type GetThreshold() { return threshold_; }

    inline sample_type GetKnee() { return knee_; }
    inline sample_type GetRatio() { return ratio_; }
    inline sample_type GetGainReductionDb() { return gain_reduction_; }

    inline sample_type Process(sample_type sample)
    {
        if(topology_ == Topology::FEEDFORWARD) { env_follower_.Process(sample); }

        const auto env = amp_to_db<sample_type>(env_follower_.Value());

        gain_reduction_ = 0;
        // if(env > threshold_)
        // {
        //     auto offset = threshold_ - -65.f;
        //     gain_reduction_ = polynomial(COEFFS, env - offset) + offset;
        // }

        if(knee_ > 0.f && env > knee_lower_bound_ && env < knee_upper_bound_)
        {
            auto slope = slope_ * ((env - knee_lower_bound_) / knee_) * 0.5;
            gain_reduction_ = slope * (knee_lower_bound_ - env);
        }
        else
        {
            gain_reduction_ = slope_ * (threshold_ - env);
            gain_reduction_ = std::min<sample_type>(0.f, gain_reduction_);
        }

        gain_reduction_ = std::min<sample_type>(0.f, gain_reduction_);
        sample *= mbdsp::db_to_amp<sample_type>(gain_reduction_);

        if(topology_ == Topology::FEEDBACK) { env_follower_.Process(sample); }

        return sample * mbdsp::db_to_amp<sample_type>(gain_);
    }
    sample_type Env() const { return env_follower_.Value(); }

protected:
    EnvFollower<sample_type> env_follower_;
    sample_type gain_reduction_;
    sample_type knee_;
    sample_type ratio_;
    sample_type threshold_;
    sample_type knee_lower_bound_;
    sample_type knee_upper_bound_;
    sample_type slope_;
    sample_type gain_;
    CompMode comp_mode_;
    Topology topology_;
    bool makeup_;

    inline void CalcKnee()
    {
        auto half_knee = knee_ / 2.f;
        knee_lower_bound_ = threshold_ - half_knee;
        knee_upper_bound_ = threshold_ + half_knee;
    }

    inline void CalcSlope()
    {
        if(comp_mode_ == CompMode::COMP) { slope_ = 1 - (1 / ratio_); }
        else if(comp_mode_ == CompMode::LIMIT) { slope_ = 1; }
    }

    inline void CalcMakeup()
    {
        if(makeup_) { gain_ = std::fabs(threshold_ - threshold_ / ratio_) * 0.1; }
    }
};
}  // namespace mbdsp