#pragma once
#include "../packets.h"
#include "stm_variables.h"
struct STMStateReport : ETCS_packet
{
    NID_STMSTATE_t NID_STMSTATE;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        NID_STMSTATE.copy(w);
    }
};
