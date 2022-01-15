#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>
struct TC_bigmetal_element_packet
{
    D_TRACKCOND_t D_TRACKCOND;
    L_TRACKCOND_t L_TRACKCOND;
    void copy(bit_manipulator &r)
    {
        D_TRACKCOND.copy(r);
        L_TRACKCOND.copy(r);
    }
};

struct TrackConditionBigMetalMasses : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    TC_bigmetal_element_packet element;
    N_ITER_t N_ITER;
    std::vector<TC_bigmetal_element_packet> elements;
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