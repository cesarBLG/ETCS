#pragma once
#include <vector>
#include "types.h"
#include "packets.h"
struct GradientElement
{
    D_GRADIENT_t D_GRADIENT;
    Q_GDIR_t Q_GDIR;
    G_A_t G_A;
    void copy(bit_manipulator &r)
    {
        D_GRADIENT.copy(r);
        Q_GDIR.copy(r);
        G_A.copy(r);
    }
};
struct GradientProfile : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    GradientElement element;
    N_ITER_t N_ITER;
    std::vector<GradientElement> elements;
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