#pragma once
#include "packets.h"
struct InfillLocationReference : ETCS_directional_packet
{
    Q_NEWCOUNTRY_t Q_NEWCOUNTRY;
    NID_C_t NID_C;
    NID_BG_t NID_BG;
    InfillLocationReference(bit_read_temp &r )
    {
        r.read(&NID_PACKET);
        r.read(&Q_DIR);
        r.read(&L_PACKET);
        r.read(&Q_NEWCOUNTRY);
        if (Q_NEWCOUNTRY)
            r.read(&NID_C);
        r.read(&NID_BG);
    }
    InfillLocationReference *create(bit_read_temp &r) override
    {
        return new InfillLocationReference(r);
    }
};