#pragma once
#include "../packets.h"
#include "stm_variables.h"
struct STMLanguage : ETCS_packet
{
    STMLanguage()
    {
        NID_PACKET.rawdata = 30;
    }
    NID_DRV_LANGUAGE_t NID_DRV_LANGUAGE;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        NID_DRV_LANGUAGE.copy(w);
    }
};
