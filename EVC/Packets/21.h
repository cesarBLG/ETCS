#pragma once
#include <vector>
#include "types.h"
#include "packets.h"
struct GradientElement
{
    D_GRADIENT_t D_GRADIENT;
    Q_GDIR_t Q_GDIR;
    G_A_t G_A;
    GradientElement() {}
    GradientElement(bit_read_temp &r)
    {
        r.read(&D_GRADIENT);
        r.read(&Q_GDIR);
        r.read(&G_A);
    }
};
struct GradientProfile : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    GradientElement element;
    N_ITER_t N_ITER;
    std::vector<GradientElement> elements;
    GradientProfile() {}
    GradientProfile(bit_read_temp &r)
    {
        r.read(&NID_PACKET);
        r.read(&Q_DIR);
        r.read(&L_PACKET);
        r.read(&Q_SCALE);
        element = GradientElement(r);
        r.read(&N_ITER);
        for (int i=0; i<N_ITER; i++) {
            elements.push_back(GradientElement(r));
        }
    }
    GradientProfile *create(bit_read_temp &r) override
    {
        return new GradientProfile(r);
    }
};