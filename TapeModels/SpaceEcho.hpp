#ifndef SPACEECHO_HPP
#define SPACEECHO_HPP

#include "IRs/IR_SpaceEcho.hpp"
#include "TapeAttrs.hpp"

class SpaceEcho : public TapeAttrs
{
public:
    SpaceEcho()
    {
        speed = 7.5;

        ir = IR_SPACEECHO;
        ir_size = sizeof(IR_SPACEECHO) / sizeof(IR_SPACEECHO[0]);
        lp_min = 200;
        gain_comp_db = -16;
    }
};

#endif  // SPACEECHO_HPP
