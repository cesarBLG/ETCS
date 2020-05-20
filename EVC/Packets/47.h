#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>
struct CLTO_element_packet
{
    M_LEVELTR_t M_LEVELTR;
    NID_NTC_t NID_NTC;
    CLTO_element_packet() = default;
    CLTO_element_packet(bit_read_temp &r)
    {
        r.read(&M_LEVELTR);
        if (M_LEVELTR == 1)  
            r.read(&NID_NTC);
    }
};

struct ConditionalLevelTransitionOrder : ETCS_directional_packet
{
    CLTO_element_packet element;
    N_ITER_t N_ITER;
    std::vector<CLTO_element_packet> elements;
    ConditionalLevelTransitionOrder() = default;
    ConditionalLevelTransitionOrder(bit_read_temp &r)
    {
        r.read(&NID_PACKET);
        r.read(&Q_DIR);
        r.read(&L_PACKET);
        element = CLTO_element_packet(r);
        r.read(&N_ITER);
        for (int i=0; i<N_ITER; i++) {
            elements.push_back(CLTO_element_packet(r));
        }

    }
    ConditionalLevelTransitionOrder *create(bit_read_temp &r) override
    {
        return new ConditionalLevelTransitionOrder(r);
    }
};