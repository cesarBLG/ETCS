#pragma once
#include "../packets.h"
#include "stm_variables.h"
struct STMSpecificBrakeCommand : ETCS_packet
{
    M_TIRB_CMD_t M_TIRB_CMD;
    M_TIMSH_CMD_t M_TIMSH_CMD;
    M_TIEDCBEB_CMD_t M_TIEDCBEB_CMD;
    M_TIEDCBSB_CMD_t M_TIEDCBSB_CMD;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        M_TIRB_CMD.copy(w);
        M_TIMSH_CMD.copy(w);
        M_TIEDCBEB_CMD.copy(w);
        M_TIEDCBSB_CMD.copy(w);
    }
};
