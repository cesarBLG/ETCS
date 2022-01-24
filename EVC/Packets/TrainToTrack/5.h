#pragma once
#include "../types.h"
#include "../variables.h"
#include "../packets.h"
struct TrainRunningNumber : ETCS_packet
{
    NID_OPERATIONAL_t NID_OPERATIONAL;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        NID_OPERATIONAL.copy(w);
    }
};