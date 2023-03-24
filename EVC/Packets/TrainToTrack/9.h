#pragma once
#include "../types.h"
#include "../variables.h"
#include "../packets.h"
struct Level23TransitionInformation : ETCS_packet
{
    NID_LTRBG_t NID_LTRBG;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        NID_LTRBG.copy(w);
    }
};