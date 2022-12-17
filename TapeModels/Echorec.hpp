#ifndef ECHOREC_HPP
#define ECHOREC_HPP

#include "IRs/IR_Echorec_Lofi.hpp"
#include "TapeAttrs.hpp"

class Echorec : public TapeAttrs
{
public:
    Echorec()
    {
        float speed = 7.5;

        ir = IR_ECHOREC_LOFI;
        ir_size = sizeof(IR_ECHOREC_LOFI) / sizeof(IR_ECHOREC_LOFI[0]);

        lp_min = 1000;
        lp_max = 20000;
        gain_comp_db = -18;
    }
};

#endif  // ECHOREC_HPP
