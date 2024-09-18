#pragma once

namespace mbdsp
{
#ifdef MBDSP_USE_INPLACE_FUNCTION
#define SG14_INPLACE_FUNCTION_THROW(x) (void) 0
#include <SG14/inplace_function.h>

#ifdef MBDSP_INPLACE_FUNCTION_SIZE
template <typename signature>
using function = stdext::inplace_function<signature, MBDSP_INPLACE_FUNCTION_SIZE, MBDSP_ALIGNMENT>;
#else
template <typename signature>
using function = stdext::inplace_function<signature>;
#endif

#else
#include <functional>
template <typename signature>
using function = std::function<signature>;
#endif

struct no_op
{
    void operator()() {}
};

}  // namespace mbdsp