#ifndef DELAY_HPP
#define DELAY_HPP

#include <daisysp.h>

#include "AudioProcessor.hpp"
#include "EnvelopeFollower.hpp"
#include "SmoothedValue.hpp"

namespace mbdsp
{

template <size_t max_size, class Sample_t = float>
class Delay : public mbdsp::AudioProcessor<Sample_t>
{
public:
    using sample_type = Sample_t;

    void Init(float sample_rate, float time_smooth_ms)
    {
        fs_ = sample_rate;
        delay_.Init();
        timeVal_.Init(sample_rate, time_smooth_ms);
    }

    sample_type Process(sample_type in) override
    {
        delay_.SetDelay(timeVal_.Process());

        auto read = delay_.Read();
        float write = feedback_ * read;
        if(inputEnable_) { write += in; }

        if(proc_) { write = proc_->Process(write); }

        delay_.Write(write);

        return read;
    }

    inline void SetFeedback(float feedback) { feedback_ = feedback; }

    inline void EnableInput(bool enable) { inputEnable_ = enable; }

    inline void SetTime(float time) { timeVal_.SetTarget(mbdsp::clamp<float>(time, 0, max_size)); }

    inline float GetTime() { return timeVal_.GetCurrent(); }

    inline void Reset() { delay_.Reset(); }

    inline void SetFeedbackProcessor(AudioProcessor<sample_type>* proc) { proc_ = proc; }

private:
    float fs_;
    mbdsp::SmoothedValue<float> timeVal_;
    float feedback_ = 0;
    bool inputEnable_ = true;
    AudioProcessor<sample_type>* proc_;
    daisysp::DelayLine<float, max_size> delay_;
};

}  // namespace mbdsp

#endif  // DELAY_HPP