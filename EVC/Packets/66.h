#pragma once
#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>

struct TemporarySpeedRestrictionRevocation : ETCS_directional_packet
{
    NID_TSR_t NID_TSR;
    TemporarySpeedRestrictionRevocation() = default;
    TemporarySpeedRestrictionRevocation(bit_read_temp &r)
    {
        r.read(&NID_PACKET);
        r.read(&Q_DIR);
        r.read(&L_PACKET);
        r.read(&NID_TSR);
    }
    TemporarySpeedRestrictionRevocation *create(bit_read_temp &r) override
    {
        return new TemporarySpeedRestrictionRevocation(r);
    }
};