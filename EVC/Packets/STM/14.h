#pragma once
#include "../packets.h"
#include "stm_variables.h"
struct STMStateOrder : ETCS_packet
{
    STMStateOrder()
    {
        NID_PACKET.rawdata = 14;
    }
    NID_STMSTATEORDER_t NID_STMSTATEORDER;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        NID_STMSTATEORDER.copy(w);
    }
};
