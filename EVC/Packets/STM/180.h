#pragma once
#include "../packets.h"
#include "stm_variables.h"
#include <vector>
struct STMDataFieldResult
{
    NID_DATA_t NID_DATA;
    L_VALUE_t L_VALUE;
    std::vector<X_VALUE_t> X_VALUE;
    void copy(bit_manipulator &w)
    {
        NID_DATA.copy(w);
        L_VALUE.copy(w);
        X_VALUE.resize(L_VALUE);
        for (int i=0; i<L_VALUE; i++) {
            X_VALUE[i].copy(w);
        }
    }
};
struct STMSpecificDataEntryValues : ETCS_packet
{
    STMSpecificDataEntryValues()
    {
        NID_PACKET.rawdata = 180;
    }
    N_ITER_t N_ITER;
    std::vector<STMDataFieldResult> results;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        N_ITER.copy(w);
        results.resize(N_ITER);
        for (int i=0; i<N_ITER; i++)
        {
            results[i].copy(w);
        }
    }
};
