#pragma once
#include "packets.h"
struct VirtualBaliseCoverOrder : ETCS_directional_packet
{
    Q_VBCO_t Q_VBCO;
    NID_VBCMK_t NID_VBCMK;
    NID_C_t NID_C;
    T_VBC_t T_VBC;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_VBCO.copy(r);
        NID_VBCMK.copy(r);
        NID_C.copy(r);
        if (Q_VBCO == 1)
            T_VBC.copy(r);
    }
};
