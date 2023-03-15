#pragma once
#include "../packets.h"
#include "stm_variables.h"
#include <vector>
struct STMSpecificDataViewField
{
    NID_DATA_t NID_DATA;
    L_CAPTION_t L_CAPTION;
    std::vector<X_CAPTION_t> X_CAPTION;
    L_VALUE_t L_VALUE;
    std::vector<X_VALUE_t> X_VALUE;
    void copy(bit_manipulator &w)
    {
        NID_DATA.copy(w);
        L_CAPTION.copy(w);
        X_CAPTION.resize(L_CAPTION);
        for (int i=0; i<L_CAPTION; i++) {
            X_CAPTION[i].copy(w);
        }
        L_VALUE.copy(w);
        X_VALUE.resize(L_VALUE);
        for (int i=0; i<L_VALUE; i++) {
            X_VALUE[i].copy(w);
        }
    }
};
struct STMSpecificDataView : ETCS_packet
{
    Q_FOLLOWING_t Q_FOLLOWING;
    N_ITER_t N_ITER;
    std::vector<STMSpecificDataViewField> fields;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        Q_FOLLOWING.copy(w);
        N_ITER.copy(w);
        fields.resize(N_ITER);
        for (int i=0; i<N_ITER; i++)
        {
            fields[i].copy(w);
        }
    }
};
