#pragma once
#include "../packets.h"
#include "stm_variables.h"
struct STMDataEntryFlag : ETCS_packet
{
    STMDataEntryFlag()
    {
        NID_PACKET.rawdata = 184;
    }
    M_DATAENTRYFLAG_t M_DATAENTRYFLAG;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        M_DATAENTRYFLAG.copy(w);
    }
};
