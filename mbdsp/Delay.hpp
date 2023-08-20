#ifndef DELAY_HPP
#define DELAY_HPP

#include "AudioProcessor.hpp"
#include "DelayLine.hpp"
#include "EnvelopeFollower.hpp"
#include "SmoothedValue.hpp"

namespace mbdsp
{

template <class T = float>
class Delay : public mbdsp::AudioProcessor<T>
{
public:
    using sample_type = T;

    void Init(float sample_rate, size_t max_delay, float time_smooth_ms)
    {
        fs_ = sample_rate;
        delay_.Init(max_delay);
        time_smoothed_.Init(sample_rate, time_smooth_ms);
    }

    sample_type Process(sample_type in) override
    {
        const auto read = delay_.Read(time_smoothed_.Process());
        float write = feedback_ * read;
        if(inputEnable_) { write += in; }

        if(proc_) { write = proc_->Process(write); }

        delay_.Write(write);

        return read;
    }

    inline void SetFeedback(float feedback) { feedback_ = feedback; }

    inline void EnableInput(bool enable) { inputEnable_ = enable; }

    inline void SetTime(float time)
    {
        time_smoothed_.SetTarget(mbdsp::clamp<float>(time, 0, delay_.MaxDelay()));
    }

    inline float GetTime() { return time_smoothed_.GetCurrent(); }

    inline void Reset() { delay_.Reset(); }

    inline void SetFeedbackProcessor(AudioProcessor<sample_type>* proc) { proc_ = proc; }

private:
    DelayLine<sample_type> delay_;
    SmoothedValue<float> time_smoothed_;
    float fs_;
    float feedback_ = 0;
    bool inputEnable_ = true;
    AudioProcessor<sample_type>* proc_;
};

}  // namespace mbdsp

#endif  // DELAY_HPP