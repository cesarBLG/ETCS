#pragma once
#include "../packets.h"
#include "stm_variables.h"
#include <vector>
struct STMButtonEventElement
{
    NID_BUTTON_t NID_BUTTON;
    Q_BUTTON_t Q_BUTTON;
    T_BUTTONEVENT_t T_BUTTONEVENT;
    void copy(bit_manipulator &w)
    {
        NID_BUTTON.copy(w);
        Q_BUTTON.copy(w);
        T_BUTTONEVENT.copy(w);
    }
};
struct STMButtonEvent : ETCS_packet
{
    N_ITER_t N_ITER;
    std::vector<STMButtonEventElement> elements;
    STMButtonEvent()
    {
        NID_PACKET.rawdata = 34;
    }
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
