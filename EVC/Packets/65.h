#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>
struct TemporarySpeedRestriction : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    NID_TSR_t NID_TSR;
    D_TSR_t D_TSR;
    L_TSR_t L_TSR;
    Q_FRONT_t Q_FRONT;
    V_TSR_t V_TSR;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_SCALE.copy(r);
        NID_TSR.copy(r);
        D_TSR.copy(r);
        L_TSR.copy(r);
        Q_FRONT.copy(r);
        V_TSR.copy(r);
    }
};