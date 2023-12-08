#pragma once
#include "variables.h"
#include "packets.h"
struct ReversingSupervisionInformation : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    D_REVERSE_t D_REVERSE;
    V_REVERSE_t V_REVERSE;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_SCALE.copy(r);
        D_REVERSE.copy(r);
        V_REVERSE.copy(r);
    }
};