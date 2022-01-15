#pragma once
#include "../types.h"
#include "../variables.h"
#include "../packets.h"
#include "../../optional.h"
#include <vector>
struct TrainDataPacket : ETCS_packet
{
    NC_CDTRAIN_t NC_CDTRAIN;
    NC_TRAIN_t NC_TRAIN;
    L_TRAIN_t L_TRAIN;
    V_MAXTRAIN_t V_MAXTRAIN;
    M_LOADINGGAUGE_t M_LOADINGGAUGE;
    M_AXLELOADCAT_t M_AXLELOADCAT;
    M_AIRTIGHT_t M_AIRTIGHT;
    N_AXLE_t N_AXLE;
    N_ITER_t N_ITERtraction;
    std::vector<M_VOLTAGE_t> M_VOLTAGEs;
    std::vector<NID_CTRACTION_t> NID_CTRACTIONs;
    N_ITER_t N_ITERntc;
    std::vector<NID_NTC_t> NID_NTCs;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        L_TRAIN.copy(w);
        V_MAXTRAIN.copy(w);
        M_LOADINGGAUGE.copy(w);
        M_AXLELOADCAT.copy(w);
        M_AIRTIGHT.copy(w);
        N_AXLE.copy(w);
        N_ITERtraction.copy(w);
        M_VOLTAGEs.resize(N_ITERtraction);
        for (int i=0; i<N_ITERtraction; i++) {
            M_VOLTAGEs[i].copy(w);
            if (M_VOLTAGEs[i] != 0) {
                NID_CTRACTIONs[i].copy(w);
            }
        }
        N_ITERntc.copy(w);
        NID_NTCs.resize(N_ITERntc);
        for (int i=0; i<N_ITERntc; i++) {
            NID_NTCs[i].copy(w);
        }
    }
};