#ifndef TAPE_HPP
#define TAPE_HPP

#include <memory>

#include <daisysp.h>

#include "CHOWTape/HysteresisProcessing.h"

#include "AudioProcessor.hpp"
#include "EnvelopeFollower.hpp"
#include "Oversampler.hpp"
#include "Utils.hpp"

#include "TapeModels/Echorec.hpp"
#include "TapeModels/SpaceEcho.hpp"
#include "TapeModels/TapeModel.hpp"

#include "DcBlocker.hpp"

namespace mbdsp
{
template <class T = float>
class Tape : public AudioProcessor<T>
{
public:
    using sample_type = T;

    void Init(float sample_rate)
    {
        os_.Init();
        os_rate_ = sample_rate * os_.GetOsFactor();
        hyst_.setSampleRate(os_rate_);
        hyst_.cook(.5, .5, .5, false);
        hyst_.reset();
        comp_.Init(3, 150, 0, 1.1, 3, os_rate_);
        comp_.SetMode(Compressor<sample_type>::CompMode::COMP);
        comp_.SetThreshold(-45);
        lp_.Init(sample_rate);
        lp_.SetFreq(5000);
        lp_.SetRes(0);
        hp_.Init(sample_rate);
        hp_.SetRes(0);
        dc_blocker_.Init(sample_rate, 35.f);
        SetModel(0);
    }

    sample_type Process(sample_type in) override
    {
        auto sample = os_.Process(in, [this](sample_type sample) {
            // Compression
            sample = comp_.Process(sample);

            // Hysteresis
            sample *= pre_gain_;
            // clip input to avoid unstable hysteresis
            sample = mbdsp::clamp<float>(sample, -10, 10);
            sample = hyst_.process<RK2>((double) sample);
            sample /= pre_gain_;

            return sample;
        });

        sample = dc_blocker_.Process(sample);

        // Loss
        lp_.Process(sample);
        sample = lp_.Low();
        hp_.Process(sample);
        sample = hp_.High();

        // IR
        if(model_->ir) { sample = fir_.Process(sample); }
        sample *= db_to_amp(model_->gain_comp_db);

        return sample * post_gain_;
    }

    inline void SetDrive(float gainDb) { pre_gain_ = mbdsp::db_to_amp(gainDb); }

    inline void SetLossFilter(float freq)
    {
        const auto& min_fc = model_->lp_min;
        const auto& max_fc = model_->lp_max;

        freq = mbdsp::remap_exp(freq, min_fc, max_fc);
        lp_.SetFreq(freq);

        // add a bit of gain to compensate for loudness lost due to filter
        auto gain_db = mbdsp::remap(max_fc - freq, min_fc, max_fc, 0.f, 6.f);
        post_gain_ = mbdsp::db_to_amp(gain_db);
    }

    inline void SetHpCutoff(float freq) { hp_.SetFreq(freq); }

    inline void SetModel(std::shared_ptr<const TapeModel> model)
    {
        if(*model == *model_) { return; }
        model_ = model;

        UpdateModel();
    }

    inline const auto& GetModel() const { return model_; }

private:
    void UpdateModel()
    {
        if(model_->ir) { fir_.SetIR(model_->ir, model_->ir_size, true); }
        hp_.SetFreq(model_->hp);
    }

    Oversampler<sample_type> os_;
    HysteresisProcessing hyst_;
    Compressor<sample_type> comp_;
    daisysp::Svf lp_;
    daisysp::Svf hp_;
    daisysp::FIRFilterImplGeneric<512, 4> fir_;
    DcBlocker<sample_type> dc_blocker_;
    std::shared_ptr<const TapeModel> model_;
    float post_gain_;
    float pre_gain_;
    float os_rate_;
};
}  // namespace mbdsp

#endif  // TAPE_HPP