#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>

struct RadioNetworkRegistration : ETCS_directional_packet
{
    NID_MN_t NID_MN;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        NID_MN.copy(r);
    }
};