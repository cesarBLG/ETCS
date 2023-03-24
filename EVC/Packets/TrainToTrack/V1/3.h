#pragma once
#include "../../packets.h"
#include <vector>
namespace V1
{
struct OnboardTelephoneNumbers : ETCS_packet
{
    OnboardTelephoneNumbers()
    {
        NID_PACKET.rawdata = 3;
    }
    NID_RADIO_t NID_RADIO;
    N_ITER_t N_ITER;
    std::vector<NID_RADIO_t> NID_RADIOs;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        NID_RADIO.copy(w);
        N_ITER.copy(w);
        NID_RADIOs.resize(N_ITER);
        for (int i=0; i<N_ITER; i++) {
            NID_RADIOs[i].copy(w);
        }
    }
};
}