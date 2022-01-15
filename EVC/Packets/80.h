#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>
struct MP_element_packet
{
    D_MAMODE_t D_MAMODE;
    M_MAMODE_t M_MAMODE;
    V_MAMODE_t V_MAMODE;
    L_MAMODE_t L_MAMODE;
    L_ACKMAMODE_t L_ACKMAMODE;
    Q_MAMODE_t Q_MAMODE;
    void copy(bit_manipulator &r)
    {
        D_MAMODE.copy(r);
        M_MAMODE.copy(r);
        V_MAMODE.copy(r);
        L_MAMODE.copy(r);
        L_ACKMAMODE.copy(r);
        Q_MAMODE.copy(r);
    }
};

struct ModeProfile : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    MP_element_packet element;
    N_ITER_t N_ITER;
    std::vector<MP_element_packet> elements;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_SCALE.copy(r);
        element.copy(r);
        N_ITER.copy(r);
        elements.resize(N_ITER);
        for (int i=0; i<N_ITER; i++) {
            elements[i].copy(r);
        }

    }
};