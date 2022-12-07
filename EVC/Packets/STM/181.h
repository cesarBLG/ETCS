#pragma once
#include "../packets.h"
#include "stm_variables.h"
struct STMSpecificDataNeed : ETCS_packet
{
    Q_DATAENTRY_t Q_DATAENTRY;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        Q_DATAENTRY.copy(w);
    }
};
