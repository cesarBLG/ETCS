#pragma once
#include "../packets.h"
#include "stm_variables.h"
struct ETCSStatusData : ETCS_packet
{
    M_LEVEL_t M_LEVEL;
    NID_NTC_t NID_NTC;
    M_MODE_t M_MODESTM;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        M_LEVEL.copy(w);
        if (M_LEVEL.rawdata == 1)
            NID_NTC.copy(w);
        M_MODESTM.copy(w);
    }
};
