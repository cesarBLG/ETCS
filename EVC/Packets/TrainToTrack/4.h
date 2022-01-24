#pragma once
#include "../types.h"
#include "../variables.h"
#include "../packets.h"
struct ErrorReporting : ETCS_packet
{
    M_ERROR_t M_ERROR;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        M_ERROR.copy(w);
    }
};