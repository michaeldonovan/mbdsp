#pragma once

namespace mbdsp
{
#ifdef MBDSP_USE_INPLACE_FUNCTION
#define SG14_INPLACE_FUNCTION_THROW(x) (void) 0
#include <SG14/inplace_function.h>

#ifdef MBDSP_FUNCTION_SIZE
template <typename signature>
using function = stdext::inplace_function<signature, MBDSP_FUNCTION_SIZE>;
#else
template <typename signature>
using function = stdext::inplace_function<signature>;
#endif

#else
#include <functional>
template <typename signature>
using function = std::function<signature>;
#endif
}  // namespace mbdsp