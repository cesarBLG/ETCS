#pragma once
#include "../packets.h"
struct STMNationalTrip : ETCS_packet
{
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
    }
};
