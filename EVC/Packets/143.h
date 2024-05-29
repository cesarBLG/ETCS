#pragma once
#include "variables.h"
#include "packets.h"
#include <vector>

struct SessionManagementNeighbourRIU : ETCS_directional_packet
{
    Q_RIU_t Q_RIU;
    NID_C_t NID_C;
    NID_RIU_t NID_RIU;
    NID_RADIO_t NID_RADIO;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_RIU.copy(r);
        NID_C.copy(r);
        NID_RIU.copy(r);
        NID_RADIO.copy(r);
    }
};