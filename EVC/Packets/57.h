#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
struct MovementAuthorityRequestParameters : ETCS_directional_packet
{
    T_MAR_t T_MAR;
    T_TIMEOUTRQST_t T_TIMEOUTRQST;
    T_CYCRQST_t T_CYCRQST;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        T_MAR.copy(r);
        T_TIMEOUTRQST.copy(r);
        T_CYCRQST.copy(r);
    }
};