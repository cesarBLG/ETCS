#pragma once
#include "variables.h"
#include "packets.h"
struct ReversingAreaInformation : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    D_STARTREVERSEAREA_t D_STARTREVERSEAREA;
    L_REVERSEAREA_t L_REVERSEAREA;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_SCALE.copy(r);
        D_STARTREVERSEAREA.copy(r);
        L_REVERSEAREA.copy(r);
    }
};