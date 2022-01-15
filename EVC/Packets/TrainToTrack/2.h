#pragma once
#include "../types.h"
#include "../variables.h"
#include "../packets.h"
#include <vector>

struct OnboardSupportedSystemVersion : ETCS_packet
{
    M_VERSION_t M_VERSION;
    N_ITER_t N_ITER;
    std::vector<M_VERSION_t> M_VERSIONs;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        M_VERSION.copy(w);
        N_ITER.copy(w);
        M_VERSIONs.resize(N_ITER);
        for (int i=0; i<N_ITER; i++) {
            M_VERSIONs[i].copy(w);
        }
    }
};