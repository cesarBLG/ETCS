#pragma once
#include "../packets.h"
#include "../39.h"
#include "variables.h"
#include <vector>
namespace V1
{
struct TrackConditionChangeTractionSystem : ::TrackConditionChangeTractionSystem
{
    M_TRACTION_t M_TRACTION;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_SCALE.copy(r);
        D_TRACTION.copy(r);
        M_TRACTION.copy(r);
    }
};
}