#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>
struct LTO_element_packet
{
    M_LEVELTR_t M_LEVELTR;
    NID_NTC_t NID_NTC;
    L_ACKLEVELTR_t L_ACKLEVELTR;
    LTO_element_packet() = default;
    LTO_element_packet(bit_read_temp &r)
    {
        r.read(&M_LEVELTR);
        if (M_LEVELTR == 1)  
            r.read(&NID_NTC);
        r.read(&L_ACKLEVELTR);
    }
};

struct LevelTransitionOrder : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    D_LEVELTR_t D_LEVELTR;
    LTO_element_packet element;
    N_ITER_t N_ITER;
    std::vector<LTO_element_packet> elements;
    LevelTransitionOrder() = default;
    LevelTransitionOrder(bit_read_temp &r)
    {
        r.read(&NID_PACKET);
        r.read(&Q_DIR);
        r.read(&L_PACKET);
        r.read(&Q_SCALE);
        r.read(&D_LEVELTR);
        element = LTO_element_packet(r);
        r.read(&N_ITER);
        for (int i=0; i<N_ITER; i++) {
            elements.push_back(LTO_element_packet(r));
        }

    }
    LevelTransitionOrder *create(bit_read_temp &r) override
    {
        return new LevelTransitionOrder(r);
    }
};