#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
struct InhibitionOfRevocableTSRL23 : ETCS_directional_packet
{
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
    }
};