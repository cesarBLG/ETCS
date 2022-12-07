#pragma once
#include "../packets.h"
#include "stm_variables.h"
#include "35.h"
#include <vector>
struct STMButtonElement
{
    NID_BUTTON_t NID_BUTTON;
    NID_BUTPOS_t NID_BUTPOS;
    NID_ICON_t NID_ICON;
    M_BUT_ATTRIB_t M_BUT_ATTRIB;
    L_CAPTION_t L_CAPTION;
    std::vector<X_CAPTION_t> X_CAPTION;
    void copy(bit_manipulator &w)
    {
        NID_BUTTON.copy(w);
        NID_BUTPOS.copy(w);
        NID_ICON.copy(w);
        M_BUT_ATTRIB.copy(w);
        L_CAPTION.copy(w);
        X_CAPTION.resize(L_CAPTION);
        for (int i=0; i<L_CAPTION; i++) {
            X_CAPTION[i].copy(w);
        }
    }
};
struct STMButtonRequest : ETCS_packet
{
    N_ITER_t N_ITER;
    std::vector<STMButtonElement> elements;
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
