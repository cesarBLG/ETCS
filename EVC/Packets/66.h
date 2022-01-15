#pragma once
#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>

struct TemporarySpeedRestrictionRevocation : ETCS_directional_packet
{
    NID_TSR_t NID_TSR;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        NID_TSR.copy(r);
    }
};