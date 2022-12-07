#pragma once
#include "../packets.h"
#include "stm_variables.h"
struct STMStateRequest : ETCS_packet
{
    NID_STMSTATEREQUEST_t NID_STMSTATEREQUEST;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        NID_STMSTATEREQUEST.copy(w);
    }
};
