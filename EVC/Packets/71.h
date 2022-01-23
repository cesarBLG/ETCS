#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
struct AdhesionFactor : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    D_ADHESION_t D_ADHESION;
    L_ADHESION_t L_ADHESION;
    M_ADHESION_t M_ADHESION;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_SCALE.copy(r);
        D_ADHESION.copy(r);
        L_ADHESION.copy(r);
        M_ADHESION.copy(r);
    }
};