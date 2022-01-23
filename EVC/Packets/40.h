#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
struct TrackConditionChangeCurrentConsumption : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    D_CURRENT_t D_CURRENT;
    M_CURRENT_t M_CURRENT;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_SCALE.copy(r);
        D_CURRENT.copy(r);
        M_CURRENT.copy(r);
    }
};