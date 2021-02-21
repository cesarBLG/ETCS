#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>
struct TC_bigmetal_element_packet
{
    D_TRACKCOND_t D_TRACKCOND;
    L_TRACKCOND_t L_TRACKCOND;
    
    TC_bigmetal_element_packet() = default;
    TC_bigmetal_element_packet(bit_read_temp &r)
    {
        r.read(&D_TRACKCOND);
        r.read(&L_TRACKCOND);
    }
};

struct TrackConditionBigMetalMasses : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    TC_bigmetal_element_packet element;
    N_ITER_t N_ITER;
    std::vector<TC_bigmetal_element_packet> elements;
    TrackConditionBigMetalMasses() = default;
    TrackConditionBigMetalMasses(bit_read_temp &r)
    {
        r.read(&NID_PACKET);
        r.read(&Q_DIR);
        r.read(&L_PACKET);
        r.read(&Q_SCALE);
        element = TC_bigmetal_element_packet(r);
        r.read(&N_ITER);
        for (int i=0; i<N_ITER; i++) {
            elements.push_back(TC_bigmetal_element_packet(r));
        }
    }
    TrackConditionBigMetalMasses *create(bit_read_temp &r) override
    {
        return new TrackConditionBigMetalMasses(r);
    }
};