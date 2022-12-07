#pragma once
#include "../packets.h"
struct STMOverrideActivation : ETCS_packet
{
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
    }
};
