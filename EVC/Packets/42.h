#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>

struct SessionManagement : ETCS_directional_packet
{
    Q_RBC_t Q_RBC;
    NID_C_t NID_C;
    NID_RBC_t NID_RBC;
    NID_RADIO_t NID_RADIO;
    NID_SLEEPSESSION_t NID_SLEEPSESSION;

    SessionManagement() = default;
    SessionManagement(bit_read_temp &r)
    {
        r.read(&NID_PACKET);
        r.read(&Q_DIR);
        r.read(&L_PACKET);
        r.read(&Q_RBC);
        r.read(&NID_C);
        r.read(&NID_RBC);
        r.read(&NID_RADIO);
        r.read(&NID_SLEEPSESSION);
    }
    SessionManagement *create(bit_read_temp &r) override
    {
        return new SessionManagement(r);
    }
};