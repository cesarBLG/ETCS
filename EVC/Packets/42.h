#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>

struct SessionManagement : ETCS_directional_packet
{
    Q_RBC_t Q_RBC;
    NID_C_t NID_C;
    NID_RBC_t NID_RBC;
    NID_RADIO_t NID_RADIO;
    Q_SLEEPSESSION_t Q_SLEEPSESSION;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_RBC.copy(r);
        NID_C.copy(r);
        NID_RBC.copy(r);
        NID_RADIO.copy(r);
        Q_SLEEPSESSION.copy(r);
    }
};