#pragma once
#include "variables.h"
#include "packets.h"
struct TrackAheadFreeTransition : ETCS_directional_packet
{
    Q_NEWCOUNTRY_t Q_NEWCOUNTRY;
    NID_C_t NID_C;
    NID_BG_t NID_BG;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_NEWCOUNTRY.copy(r);
        if (Q_NEWCOUNTRY)
            NID_C.copy(r);
        NID_BG.copy(r);
    }
};