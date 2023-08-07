#pragma once
#include "../packets.h"
#include "stm_variables.h"
struct STMSystemSpeed : ETCS_packet
{
    V_STMSYS_t V_STMSYS;
    D_STMSYS_t D_STMSYS;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        V_STMSYS.copy(w);
        D_STMSYS.copy(w);
    }
};
