#pragma once
#include "../packets.h"
#include "stm_variables.h"
struct STMTrainCommand : ETCS_packet
{
    M_TIPANTO_CMD_t M_TIPANTO_CMD;
    M_TIFLAP_CMD_t M_TIFLAP_CMD;
    M_TIMS_CMD_t M_TIMS_CMD;
    M_TITR_CMD_t M_TITR_CMD;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        M_TIPANTO_CMD.copy(w);
        M_TIFLAP_CMD.copy(w);
        M_TIMS_CMD.copy(w);
        M_TITR_CMD.copy(w);
    }
};
