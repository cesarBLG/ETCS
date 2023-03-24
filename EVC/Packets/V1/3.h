#pragma once
#include "../packets.h"
#include <vector>
namespace V1
{
struct NationalValues : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    D_VALIDNV_t D_VALIDNV;
    N_ITER_t N_ITER_c;
    std::vector<NID_C_t> NID_Cs;
    V_NVSHUNT_t V_NVSHUNT;
    V_NVSTFF_t V_NVSTFF;
    V_NVONSIGHT_t V_NVONSIGHT;
    V_NVUNFIT_t V_NVUNFIT;
    V_NVREL_t V_NVREL;
    D_NVROLL_t D_NVROLL;
    Q_NVSBTSMPERM_t Q_NVSBTSMPERM;
    Q_NVEMRRLS_t Q_NVEMRRLS;
    V_NVALLOWOVTRP_t V_NVALLOWOVTRP;
    V_NVSUPOVTRP_t V_NVSUPOVTRP;
    D_NVOVTRP_t D_NVOVTRP;
    T_NVOVTRP_t T_NVOVTRP;
    D_NVPOTRP_t D_NVPOTRP;
    M_NVCONTACT_t M_NVCONTACT;
    T_NVCONTACT_t T_NVCONTACT;
    M_NVDERUN_t M_NVDERUN;
    D_NVSTFF_t D_NVSTFF;
    Q_NVDRIVER_ADHES_t Q_NVDRIVER_ADHES;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        Q_DIR.copy(w);
        L_PACKET.copy(w);
        Q_SCALE.copy(w);
        D_VALIDNV.copy(w);
        N_ITER_c.copy(w);
        NID_Cs.resize(N_ITER_c);
        for (int i=0; i<N_ITER_c; i++) {
            NID_Cs[i].copy(w);
        }
        V_NVSHUNT.copy(w);
        V_NVSTFF.copy(w);
        V_NVONSIGHT.copy(w);
        V_NVUNFIT.copy(w);
        V_NVREL.copy(w);
        D_NVROLL.copy(w);
        Q_NVSBTSMPERM.copy(w);
        Q_NVEMRRLS.copy(w);
        V_NVALLOWOVTRP.copy(w);
        V_NVSUPOVTRP.copy(w);
        D_NVOVTRP.copy(w);
        T_NVOVTRP.copy(w);
        D_NVPOTRP.copy(w);
        M_NVCONTACT.copy(w);
        T_NVCONTACT.copy(w);
        M_NVDERUN.copy(w);
        D_NVSTFF.copy(w);
        Q_NVDRIVER_ADHES.copy(w);
    }
};
}