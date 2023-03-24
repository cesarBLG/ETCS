#pragma once
#include "variables.h"
#include "packets.h"
struct LSSMAToggleOrder : ETCS_directional_packet
{
    Q_LSSMA_t Q_LSSMA;
    T_LSSMA_t T_LSSMA;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_LSSMA.copy(r);
        if (Q_LSSMA == 1)
            T_LSSMA.copy(r);
    }
};