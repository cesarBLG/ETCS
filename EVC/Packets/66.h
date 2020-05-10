#pragma once
#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>

struct TemporarySpeedRevocation : ETCS_directional_packet
{
    NID_TSR_t NID_TSR;
    TemporarySpeedRevocation() = default;
    TemporarySpeedRevocation(bit_read_temp &r)
    {
        r.read(&NID_PACKET);
        r.read(&Q_DIR);
        r.read(&L_PACKET);
        r.read(&NID_TSR);
    }
    TemporarySpeedRevocation *create(bit_read_temp &r) override
    {
        return new TemporarySpeedRevocation(r);
    }
};