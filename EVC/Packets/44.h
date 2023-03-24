#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>

struct DataOutsideERTMS : ETCS_directional_packet
{
    NID_XUSER_t NID_XUSER;
    NID_NTC_t NID_NTC;
    std::vector<unsigned char> bits;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        NID_XUSER.copy(r);
        if (NID_XUSER == 102)
            NID_NTC.copy(r);
        if (r.write_mode) {
        } else {
        }
    }
};