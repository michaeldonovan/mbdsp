#ifndef ECHOREC_HPP
#define ECHOREC_HPP

#include "IRs/IR_Echorec_Hifi.hpp"
#include "TapeModel.hpp"

namespace mbdsp
{

namespace TapeModels
{

class Echorec : public TapeModel
{
public:
    Echorec()
    {
        using namespace detail;

        speed = 7.5;

        ir = IR_ECHOREC_HIFI;
        ir_size = sizeof(IR_ECHOREC_HIFI) / sizeof(IR_ECHOREC_HIFI[0]);

        lp_min = 1000;
        lp_max = 20000;
        gain_comp_db = -16;
    }
};

}  // namespace TapeModels

}  // namespace mbdsp

#endif  // ECHOREC_HPP
