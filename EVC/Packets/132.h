#pragma once
#include "variables.h"
#include "packets.h"
#include <vector>

struct DangerForShunting : ETCS_directional_packet
{
    Q_ASPECT_t Q_ASPECT;
    DangerForShunting() = default;
    DangerForShunting(bit_read_temp &r)
    {
        r.read(&NID_PACKET);
        r.read(&Q_DIR);
        r.read(&L_PACKET);
        r.read(&Q_ASPECT);
    }
    DangerForShunting *create(bit_read_temp &r) override
    {
        return new DangerForShunting(r);
    }
};