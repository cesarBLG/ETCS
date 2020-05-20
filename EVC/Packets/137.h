#pragma once
#include "variables.h"
#include "packets.h"
#include <vector>

struct StopIfInSR : ETCS_directional_packet
{
    Q_SRSTOP_t Q_SRSTOP;
    StopIfInSR() = default;
    StopIfInSR(bit_read_temp &r)
    {
        r.read(&NID_PACKET);
        r.read(&Q_DIR);
        r.read(&L_PACKET);
        r.read(&Q_SRSTOP);
    }
    StopIfInSR *create(bit_read_temp &r) override
    {
        return new StopIfInSR(r);
    }
};