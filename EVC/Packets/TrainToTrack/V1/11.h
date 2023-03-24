#pragma once
#include "../11.h"
#include "../../V1/variables.h"
#include <vector>
namespace V1
{
struct TrainDataPacket : ::TrainDataPacket
{
    NID_OPERATIONAL_t NID_OPERATIONAL;
    M_AXLELOAD_t M_AXLELOAD;
    std::vector<M_TRACTION_t> M_TRACTIONs;
    TrainDataPacket()
    {
        NID_PACKET.rawdata = 11;
    }
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        NID_OPERATIONAL.copy(w);
        NC_TRAIN.copy(w);
        L_TRAIN.copy(w);
        V_MAXTRAIN.copy(w);
        M_LOADINGGAUGE.copy(w);
        M_AXLELOAD.copy(w);
        M_AIRTIGHT.copy(w);
        N_ITERtraction.copy(w);
        M_TRACTIONs.resize(N_ITERtraction);
        for (int i=0; i<N_ITERtraction; i++) {
            M_TRACTIONs[i].copy(w);
        }
        N_ITERntc.copy(w);
        NID_NTCs.resize(N_ITERntc);
        for (int i=0; i<N_ITERntc; i++) {
            NID_NTCs[i].copy(w);
        }
    }
};
}