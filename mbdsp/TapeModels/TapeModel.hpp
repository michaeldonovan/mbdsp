#ifndef TapeModel_HPP
#define TapeModel_HPP

#include <cstddef>

#include "mbdsp/Utils.hpp"

namespace mbdsp
{

class TapeModel
{
public:
    float speed = 15;
    float spacing = .1;
    float thickness = .1;
    float gap = 1;
    float azimuth = 0;

    float flutter_freq;
    float wow_freq;

    float const* ir = nullptr;
    size_t ir_size = 0;

    float hp = 100;
    float lp_min = 1000;
    float lp_max = 20000;

    float gain_comp_db = 0;

    auto operator<=>(const TapeModel&) const = default;
};

}  // namespace mbdsp

#endif  // TapeModel_HPP