#pragma once
#include "../packets.h"
#include "stm_variables.h"
struct STMSpecificDataViewRequest : ETCS_packet
{
    STMSpecificDataViewRequest()
    {
        NID_PACKET.rawdata = 182;
    }
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
    }
};
