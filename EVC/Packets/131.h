#pragma once
#include "variables.h"
#include "packets.h"
struct RBCTransitionOrder : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    D_RBCTR_t D_RBCTR;
    NID_C_t NID_C;
    NID_RBC_t NID_RBC;
    NID_RADIO_t NID_RADIO;
    Q_SLEEPSESSION_t Q_SLEEPSESSION;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_SCALE.copy(r);
        D_RBCTR.copy(r);
        NID_C.copy(r);
        NID_RBC.copy(r);
        NID_RADIO.copy(r);
        Q_SLEEPSESSION.copy(r);
    }
};