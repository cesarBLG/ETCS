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
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_SCALE.copy(r);
        NID_LX.copy(r);
        D_LX.copy(r);
        L_LX.copy(r);
        Q_LXSTATUS.copy(r);
        if(Q_LXSTATUS == 1) {
            V_LX.copy(r);
            Q_STOPLX.copy(r);
            if(Q_STOPLX == 1) {
                L_STOPLX.copy(r);
            }
        }
    }
};