#pragma once
#include "packets.h"
#include <vector>
struct NationalValues : ETCS_directional_packet
{
    D_VALIDNV_t D_VALIDNV;
    NID_C_t NID_C;
    N_ITER_t N_ITER;
    std::vector<NID_C_t> NID_Cs;
    V_NVSHUNT_t V_NVSHUNT;
    V_NVSTFF_t V_NVSTFF;
    V_NVONSIGHT_t V_NVONSIGHT;
    V_NVLIMSUPERV_t V_NVLIMSUPERV;
    V_NVUNFIT_t V_NVUNFIT;
    V_NVREL_t V_NVREL;
}