#pragma once
#include "packets.h"
struct DefaultGradientTSR : ETCS_directional_packet
{
    Q_GDIR_t Q_GDIR;
    G_TSR_t G_TSR;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_GDIR.copy(r);
        G_TSR.copy(r);
    }
};