#pragma once
#include "packets.h"
#include <vector>
struct KVINT_step_element
{
    V_NVKVINT_t V_NVKVINT;
    M_NVKVINT_t M_NVKVINT1;
    M_NVKVINT_t M_NVKVINT2;
    void copy(bit_manipulator &w, Q_NVKVINTSET_t Q_NVKVINTSET)
    {
        V_NVKVINT.copy(w);
        M_NVKVINT1.copy(w);
        if (Q_NVKVINTSET == 1) M_NVKVINT2.copy(w);
    }
};
struct KVINT_element
{
    Q_NVKVINTSET_t Q_NVKVINTSET;
    A_NVP12_t A_NVP12;
    A_NVP23_t A_NVP23;
    KVINT_step_element element;
    N_ITER_t N_ITER;
    std::vector<KVINT_step_element> elements;
    void copy(bit_manipulator &w)
    {
        Q_NVKVINTSET.copy(w);
        if (Q_NVKVINTSET == 1) {
            A_NVP12.copy(w);
            A_NVP23.copy(w);
        }
        element.copy(w, Q_NVKVINTSET);
        N_ITER.copy(w);
        elements.resize(N_ITER);
        for (int i=0; i<N_ITER; i++) {
            elements[i].copy(w, Q_NVKVINTSET);
        }
    }
};
struct KRINT_element
{
    L_NVKRINT_t L_NVKRINT;
    M_NVKRINT_t M_NVKRINT;
    void copy(bit_manipulator &w)
    {
        L_NVKRINT.copy(w);
        M_NVKRINT.copy(w);
    }
};
struct NationalValues : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    D_VALIDNV_t D_VALIDNV;
    NID_C_t NID_C;
    N_ITER_t N_ITER_c;
    std::vector<NID_C_t> NID_Cs;
    V_NVSHUNT_t V_NVSHUNT;
    V_NVSTFF_t V_NVSTFF;
    V_NVONSIGHT_t V_NVONSIGHT;
    V_NVLIMSUPERV_t V_NVLIMSUPERV;
    V_NVUNFIT_t V_NVUNFIT;
    V_NVREL_t V_NVREL;
    D_NVROLL_t D_NVROLL;
    Q_NVSBTSMPERM_t Q_NVSBTSMPERM;
    Q_NVEMRRLS_t Q_NVEMRRLS;
    Q_NVGUIPERM_t Q_NVGUIPERM;
    Q_NVSBFBPERM_t Q_NVSBFBPERM;
    Q_NVINHSMICPERM_t Q_NVINHSMICPERM;
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
    A_NVMAXREDADH1_t A_NVMAXREDADH1;
    A_NVMAXREDADH2_t A_NVMAXREDADH2;
    A_NVMAXREDADH3_t A_NVMAXREDADH3;
    Q_NVLOCACC_t Q_NVLOCACC;
    M_NVAVADH_t M_NVAVADH;
    M_NVEBCL_t M_NVEBCL;
    Q_NVKINT_t Q_NVKINT;
    KVINT_element element_kv;
    N_ITER_t N_ITER_kv;
    std::vector<KVINT_element> elements_kv;
    KRINT_element element_kr;
    N_ITER_t N_ITER_kr;
    std::vector<KRINT_element> elements_kr;
    M_NVKTINT_t M_NVKTINT;
    bool adhesion_nv_provided = true;
    bool version_2_provided = true;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        Q_DIR.copy(w);
        L_PACKET.copy(w);
        Q_SCALE.copy(w);
        D_VALIDNV.copy(w);
        NID_C.copy(w);
        N_ITER_c.copy(w);
        NID_Cs.resize(N_ITER_c);
        for (int i=0; i<N_ITER_c; i++) {
            NID_Cs[i].copy(w);
        }
        V_NVSHUNT.copy(w);
        V_NVSTFF.copy(w);
        V_NVONSIGHT.copy(w);
        V_NVLIMSUPERV.copy(w);
        V_NVUNFIT.copy(w);
        V_NVREL.copy(w);
        D_NVROLL.copy(w);
        Q_NVSBTSMPERM.copy(w);
        Q_NVEMRRLS.copy(w);
        Q_NVGUIPERM.copy(w);
        Q_NVSBFBPERM.copy(w);
        Q_NVINHSMICPERM.copy(w);
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
        A_NVMAXREDADH1.copy(w);
        A_NVMAXREDADH2.copy(w);
        A_NVMAXREDADH3.copy(w);
        Q_NVLOCACC.copy(w);
        M_NVAVADH.copy(w);
        M_NVEBCL.copy(w);
        Q_NVKINT.copy(w);
        if (Q_NVKINT == 1) {
            element_kv.copy(w);
            N_ITER_kv.copy(w);
            elements_kv.resize(N_ITER_kv);
            for (int i=0; i<N_ITER_kv; i++) {
                elements_kv[i].copy(w);
            }
            element_kr.copy(w);
            N_ITER_kr.copy(w);
            elements_kr.resize(N_ITER_kr);
            for (int i=0; i<N_ITER_kr; i++) {
                elements_kr[i].copy(w);
            }
            M_NVKTINT.copy(w);
        }
    }
};
