/*****************************************************************************

        Downsampler2x4Sse.h
        Author: Laurent de Soras, 2015

Downsamples vectors of 4 float by a factor 2 the input signal, using SSE
instruction set.

This object must be aligned on a 16-byte boundary!

Template parameters:
    - NC: number of coefficients, > 0

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/

#pragma once
#if !defined(hiir_Downsampler2x4Sse_HEADER_INCLUDED)
#define hiir_Downsampler2x4Sse_HEADER_INCLUDED

#if defined(_MSC_VER)
#pragma warning(4 : 4250)
#endif

/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "hiir/StageDataSse.h"
#include "hiir/def.h"

#include <xmmintrin.h>

#include <array>

namespace hiir
{

template <int NC>
class Downsampler2x4Sse
{
    static_assert((NC > 0), "Number of coefficient must be positive.");

    /*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:
    typedef float DataType;
    static constexpr int _nbr_chn = 4;
    static constexpr int NBR_COEFS = NC;
    static constexpr double _delay = -1;

    Downsampler2x4Sse() noexcept;
    Downsampler2x4Sse(const Downsampler2x4Sse<NC> &other) = default;
    Downsampler2x4Sse(Downsampler2x4Sse<NC> &&other) = default;
    ~Downsampler2x4Sse() = default;

    Downsampler2x4Sse<NC> &operator=(const Downsampler2x4Sse<NC> &other) = default;
    Downsampler2x4Sse<NC> &operator=(Downsampler2x4Sse<NC> &&other) = default;

    void set_coefs(const double coef_arr[]) noexcept;

    hiir_FORCEINLINE __m128 process_sample(const float in_ptr[_nbr_chn * 2]) noexcept;
    hiir_FORCEINLINE __m128 process_sample(__m128 in_0, __m128 in_1) noexcept;
    void process_block(float out_ptr[], const float in_ptr[], long nbr_spl) noexcept;

    hiir_FORCEINLINE void process_sample_split(__m128 &low, __m128 &high,
                                               const float in_ptr[_nbr_chn * 2]) noexcept;
    hiir_FORCEINLINE void process_sample_split(__m128 &low, __m128 &high, __m128 in_0,
                                               __m128 in_1) noexcept;
    void process_block_split(float out_l_ptr[], float out_h_ptr[], const float in_ptr[],
                             long nbr_spl) noexcept;

    void clear_buffers() noexcept;

    /*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:
    /*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:
    typedef std::array<StageDataSse, NBR_COEFS + 2>
        Filter;  // Stages 0 and 1 contain only input memories

    Filter _filter;  // Should be the first member (thus easier to align)

    /*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:
    bool operator==(const Downsampler2x4Sse<NC> &other) const = delete;
    bool operator!=(const Downsampler2x4Sse<NC> &other) const = delete;

};  // class Downsampler2x4Sse

}  // namespace hiir

#include "hiir/Downsampler2x4Sse.hpp"

#endif  // hiir_Downsampler2x4Sse_HEADER_INCLUDED

/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
