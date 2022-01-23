#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>
struct TrackConditionChangeTractionSystem : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    D_TRACTION_t D_TRACTION;
    M_VOLTAGE_t M_VOLTAGE;
    NID_CTRACTION_t NID_CTRACTION;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_SCALE.copy(r);
        D_TRACTION.copy(r);
        M_VOLTAGE.copy(r);
        if (M_VOLTAGE != 0)
            NID_CTRACTION.copy(r);
    }
};