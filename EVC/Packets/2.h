#pragma once
#include "variables.h"
#include "packets.h"
struct SystemVersionOrder : ETCS_directional_packet
{
    M_VERSION_t M_VERSION;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        M_VERSION.copy(r);
    }
};