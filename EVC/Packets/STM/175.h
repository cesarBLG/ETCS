#pragma once
#include "../types.h"
#include "../variables.h"
#include "../packets.h"
#include "../../optional.h"
#include <vector>
struct STMTrainData : ETCS_packet
{
    NC_CDTRAIN_t NC_CDTRAIN;
    NC_TRAIN_t NC_TRAIN;
    L_TRAIN_t L_TRAIN;
    V_MAXTRAIN_t V_MAXTRAIN;
    M_LOADINGGAUGE_t M_LOADINGGAUGE;
    M_AXLELOADCAT_t M_AXLELOADCAT;
    M_AIRTIGHT_t M_AIRTIGHT;
    M_TRAINTYPE_t M_TRAINTYPE;
    N_ITER_t N_ITER;
    std::vector<M_VOLTAGE_t> M_VOLTAGEs;
    std::vector<NID_CTRACTION_t> NID_CTRACTIONs;
    STMTrainData()
    {
        NID_PACKET.rawdata = 175;
    }
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        NC_CDTRAIN.copy(w);
        NC_TRAIN.copy(w);
        L_TRAIN.copy(w);
        V_MAXTRAIN.copy(w);
        M_LOADINGGAUGE.copy(w);
        M_AXLELOADCAT.copy(w);
        M_AIRTIGHT.copy(w);
        M_TRAINTYPE.copy(w);
        N_ITER.copy(w);
        M_VOLTAGEs.resize(N_ITER);
        for (int i=0; i<N_ITER; i++) {
            M_VOLTAGEs[i].copy(w);
            if (M_VOLTAGEs[i] != 0) {
                NID_CTRACTIONs[i].copy(w);
            }
        }
    }
};