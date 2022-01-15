#pragma once
#include "variables.h"
#include "packets.h"
#include <vector>

struct DangerForShunting : ETCS_directional_packet
{
    Q_ASPECT_t Q_ASPECT;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_ASPECT.copy(r);
    }
};