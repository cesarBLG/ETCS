#pragma once
#include "variables.h"
#include "packets.h"
struct GenericLSFunctionMarker : ETCS_directional_packet
{
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
    }
};