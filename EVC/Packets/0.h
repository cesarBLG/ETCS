#pragma once
#include "packets.h"
struct VirtualBaliseCoverMarker : ETCS_packet
{
    NID_VBCMK_t NID_VBCMK;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        NID_VBCMK.copy(r);
    }
};
