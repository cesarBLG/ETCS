#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>
struct GeographicalPosition_element_packet
{
    Q_NEWCOUNTRY_t Q_NEWCOUNTRY;
    NID_C_t NID_C;
    NID_BG_t NID_BG;
    D_POSOFF_t D_POSOFF;
    Q_MPOSITION_t Q_MPOSITION;
    M_POSITION_t M_POSITION;
    void copy(bit_manipulator &r)
    {
        Q_NEWCOUNTRY.copy(r);
        if (Q_NEWCOUNTRY)
            NID_C.copy(r);
        NID_BG.copy(r);
        D_POSOFF.copy(r);
        Q_MPOSITION.copy(r);
        M_POSITION.copy(r);
    }
};

struct GeographicalPosition : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    GeographicalPosition_element_packet element;
    N_ITER_t N_ITER;
    std::vector<GeographicalPosition_element_packet> elements;
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