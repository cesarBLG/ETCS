#pragma once
#include "../packets.h"
#include "stm_variables.h"
struct STMDeleteTextMessage : ETCS_packet
{
    NID_XMESSAGE_t NID_XMESSAGE;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        NID_XMESSAGE.copy(w);
    }
};
