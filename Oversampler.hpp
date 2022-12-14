#ifndef OVERSAMPLER_HPP
#define OVERSAMPLER_HPP

#include <functional>

#include "hiir/Downsampler2xFpu.h"
#include "hiir/Upsampler2xFpu.h"

#include "AudioProcessor.hpp"

namespace mbdsp
{

// coefficients from hiir examples in oversampling.txt
static constexpr size_t N_COEFFS_2X = 12;
static const double COEFFS_2X[N_COEFFS_2X] = {
    0.017347915108876406, 0.067150480426919179, 0.14330738338179819, 0.23745131944299824,
    0.34085550201503761,  0.44601111310335906,  0.54753112652956148, 0.6423859124721446,
    0.72968928615804163,  0.81029959388029904,  0.88644514917318362, 0.96150605146543733};

template <class T = float>
class Oversampler
{
public:
    using sample_type = T;

    void Init()
    {
        upsampler_2x.set_coefs(COEFFS_2X);
        upsampler_2x.clear_buffers();
        downsampler_2x.set_coefs(COEFFS_2X);
        downsampler_2x.clear_buffers();
        // upsampler_4x.set_coefs(coeffs4x);
        // upsampler_4x.clear_buffers();
        // downsampler_4x.set_coefs(coeffs4x);
        // downsampler_4x.clear_buffers();
    }

    sample_type Process(sample_type in, std::function<sample_type(sample_type)> proc)
    {
        sample_type upBuff2x[2];
        sample_type upBuff4x[4];
        sample_type downBuff4x[4];
        sample_type downBuff2x[2];

        upsampler_2x.process_sample(upBuff2x[0], upBuff2x[1], in);
        // upsampler_4x.process_block(upBuff4x, upBuff2x, 2);
        downBuff2x[0] = proc(upBuff2x[0]);
        downBuff2x[1] = proc(upBuff2x[1]);

        // for (size_t i = 0; i < 4; ++i)
        // {
        //    downBuff4x[i] = proc(upBuff4x[i]);
        // }

        // downsampler_4x.process_block(downBuff2x, downBuff4x, 4);

        return downsampler_2x.process_sample(downBuff2x);
    }

    sample_type Process(sample_type in, AudioProcessor<T>* proc)
    {
        return Process(in, [proc, in](sample_type samp) { return proc->Process(samp); });
    }

private:
    hiir::Upsampler2xFpu<N_COEFFS_2X> upsampler_2x;
    hiir::Downsampler2xFpu<N_COEFFS_2X> downsampler_2x;
    // hiir::Upsampler2xFpu<N_COEFFS_4X> upsampler_4x;
    // hiir::Downsampler2xFpu<N_COEFFS_4X> downsampler_4x;
};

}  // namespace mbdsp

#endif  // OVERSAMPLER_HPP