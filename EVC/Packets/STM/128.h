#pragma once
#include "../packets.h"
#include "stm_variables.h"
struct STMBrakeCommand : ETCS_packet
{
    M_BIEB_CMD_t M_BIEB_CMD;
    M_BISB_CMD_t M_BISB_CMD;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        M_BIEB_CMD.copy(w);
        M_BISB_CMD.copy(w);
    }
};
