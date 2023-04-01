#pragma once
#include "../packets.h"
#include "stm_variables.h"
struct STMVersion : ETCS_packet
{
    STMVersion()
    {
        NID_PACKET.rawdata = 1;
    }
    N_VERMAJOR_t N_VERMAJOR;
    N_VERMINOR_t N_VERMINOR;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        N_VERMAJOR.copy(w);
        N_VERMINOR.copy(w);
    }
};
