#pragma once
#include "Concepts.hpp"
#include "Utils.hpp"
namespace mbdsp
{
template <concepts::numeric T = float>
class OnePoleLpf
{
public:
    using sample_type = T;
    void Init(sample_type time_constant_ms, sample_type sample_rate)
    {
        fs_ = sample_rate;
        SetTimeConstant(time_constant_ms);
    }

    void SetTimeConstant(sample_type ms)
    {
        auto x = std::exp(-1 / ms_to_samples(ms, fs_));
        a0_ = 1.f - x;
        b1_ = -x;
    }

    sample_type Process(sample_type in)
    {
        auto out = a0_ * in - b1_ * prev_;
        prev_ = out;
        return out;
    }

protected:
    sample_type fs_;
    sample_type a0_;
    sample_type b1_;
    sample_type prev_ = 0.f;
};
}  // namespace mbdsp