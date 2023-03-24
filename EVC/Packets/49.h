#pragma once
#include "packets.h"
#include <vector>
struct SHBaliseElement
{
    Q_NEWCOUNTRY_t Q_NEWCOUNTRY;
    NID_C_t NID_C;
    NID_BG_t NID_BG;
    void copy(bit_manipulator &r)
    {
        Q_NEWCOUNTRY.copy(r);
        if (Q_NEWCOUNTRY)
            NID_C.copy(r);
        NID_BG.copy(r);
    }
};
struct ListSHBalises : ETCS_directional_packet
{
    N_ITER_t N_ITER;
    std::vector<SHBaliseElement> elements;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        N_ITER.copy(r);
        elements.resize(N_ITER);
        for (int i=0; i<N_ITER; i++) {
            elements[i].copy(r);
        }
    }
};
