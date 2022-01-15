#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>
struct CLTO_element_packet
{
    M_LEVELTR_t M_LEVELTR;
    NID_NTC_t NID_NTC;
    void copy(bit_manipulator &r)
    {
        M_LEVELTR.copy(r);
        if (M_LEVELTR == 1)  
            NID_NTC.copy(r);
    }
};

struct ConditionalLevelTransitionOrder : ETCS_directional_packet
{
    CLTO_element_packet element;
    N_ITER_t N_ITER;
    std::vector<CLTO_element_packet> elements;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        element.copy(r);
        N_ITER.copy(r);
        elements.resize(N_ITER);
        for (int i=0; i<N_ITER; i++) {
            elements[i].copy(r);
        }

    }
};