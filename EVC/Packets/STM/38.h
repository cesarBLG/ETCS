#pragma once
#include "../packets.h"
#include "stm_variables.h"
#include <vector>
struct STMTextMessage : ETCS_packet
{
    NID_XMESSAGE_t NID_XMESSAGE;
    M_XATTRIBUTE_t M_XATTRIBUTE;
    Q_ACK_t Q_ACK;
    L_TEXT_t L_TEXT;
    std::vector<X_TEXT_t> X_TEXT;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        NID_XMESSAGE.copy(w);
        M_XATTRIBUTE.copy(w);
        Q_ACK.copy(w);
        L_TEXT.copy(w);
        X_TEXT.resize(L_TEXT);
        for (int i=0; i<L_TEXT; i++) {
            X_TEXT[i].copy(w);
        }
    }
};
