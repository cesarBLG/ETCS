#pragma once
#include "../packets.h"
#include "stm_variables.h"
struct STMMaxSpeed : ETCS_packet
{
    V_STMMAX_t V_STMMAX;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        V_STMMAX.copy(w);
    }
};
