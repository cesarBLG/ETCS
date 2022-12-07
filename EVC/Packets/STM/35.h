#pragma once
#include "../packets.h"
#include "stm_variables.h"
#include <vector>
struct STMIconElement
{
    NID_INDICATOR_t NID_INDICATOR;
    NID_INDPOS_t NID_INDPOS;
    NID_ICON_t NID_ICON;
    M_IND_ATTRIB_t M_IND_ATTRIB;
    L_CAPTION_t L_CAPTION;
    std::vector<X_CAPTION_t> X_CAPTION;
    void copy(bit_manipulator &w)
    {
        NID_INDICATOR.copy(w);
        NID_INDPOS.copy(w);
        NID_ICON.copy(w);
        M_IND_ATTRIB.copy(w);
        L_CAPTION.copy(w);
        X_CAPTION.resize(L_CAPTION);
        for (int i=0; i<L_CAPTION; i++) {
            X_CAPTION[i].copy(w);
        }
    }
};
struct STMIconRequest : ETCS_packet
{
    N_ITER_t N_ITER;
    std::vector<STMIconElement> elements;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        N_ITER.copy(w);
        elements.resize(N_ITER);
        for (int i=0; i<N_ITER; i++) {
            elements[i].copy(w);
        }
    }
};
