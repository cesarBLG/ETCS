#pragma once
#include "variables.h"
#include "packets.h"
struct StopIfInSR : ETCS_directional_packet
{
    Q_SRSTOP_t Q_SRSTOP;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_SRSTOP.copy(r);
    }
};