#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>

struct LevelCrossingInformation : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    NID_LX_t NID_LX;
    D_LX_t D_LX;
    L_LX_t L_LX;
    Q_LXSTATUS_t Q_LXSTATUS;
    V_LX_t V_LX;
    Q_STOPLX_t Q_STOPLX;
    L_STOPLX_t L_STOPLX;

    LevelCrossingInformation() = default;
    LevelCrossingInformation(bit_read_temp &r)
    {
        r.read(&NID_PACKET);
        r.read(&Q_DIR);
        r.read(&L_PACKET);
        r.read(&Q_SCALE);
        r.read(&NID_LX);
        r.read(&D_LX);
        r.read(&L_LX);
        r.read(&Q_LXSTATUS);
        if(Q_LXSTATUS == 1) {
            r.read(&V_LX);
            r.read(&Q_STOPLX);
            if(Q_STOPLX == 1) {
                r.read(&L_STOPLX);
            }
        }
    }
    LevelCrossingInformation *create(bit_read_temp &r) override
    {
        return new LevelCrossingInformation(r);
    }
};