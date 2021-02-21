#pragma once
#include "packets.h"
#include <vector>
struct KVINT_step_element
{
    V_NVKVINT_t V_NVKVINT;
    M_NVKVINT_t M_NVKVINT1;
    M_NVKVINT_t M_NVKVINT2;
    KVINT_step_element() {}
    KVINT_step_element(bit_read_temp &r, Q_NVKVINTSET_t Q_NVKVINTSET)
    {
        r.read(&V_NVKVINT);
        r.read(&M_NVKVINT1);
        if (Q_NVKVINTSET == 1) r.read(&M_NVKVINT2);
    }
    void serialize(bit_write &w, Q_NVKVINTSET_t Q_NVKVINTSET)
    {
        w.write(&V_NVKVINT);
        w.write(&M_NVKVINT1);
        if (Q_NVKVINTSET == 1) w.write(&M_NVKVINT2);
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
    KVINT_element() {}
    KVINT_element(bit_read_temp &r)
    {
        r.read(&Q_NVKVINTSET);
        if (Q_NVKVINTSET == 1) {
            r.read(&A_NVP12);
            r.read(&A_NVP23);
        }
        element = KVINT_step_element(r, Q_NVKVINTSET);
        r.read(&N_ITER);
        for (int i=0; i<N_ITER; i++) {
            elements.push_back(KVINT_step_element(r, Q_NVKVINTSET));
        }
    }
    void serialize(bit_write &w)
    {
        w.write(&Q_NVKVINTSET);
        if (Q_NVKVINTSET == 1) {
            w.write(&A_NVP12);
            w.write(&A_NVP23);
        }
        element.serialize(w, Q_NVKVINTSET);
        w.write(&N_ITER);
        for (int i=0; i<N_ITER; i++) {
            elements[i].serialize(w, Q_NVKVINTSET);
        }
    }
};
struct KRINT_element
{
    L_NVKRINT_t L_NVKRINT;
    M_NVKRINT_t M_NVKRINT;
    KRINT_element() {}
    KRINT_element(bit_read_temp &r)
    {
        r.read(&L_NVKRINT);
        r.read(&M_NVKRINT);
    }
    void serialize(bit_write &w)
    {
        w.write(&L_NVKRINT);
        w.write(&M_NVKRINT);
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
    NationalValues() {}
    NationalValues(bit_read_temp &r)
    {
        r.read(&NID_PACKET);
        r.read(&Q_DIR);
        r.read(&L_PACKET);
        r.read(&Q_SCALE);
        r.read(&D_VALIDNV);
        r.read(&NID_C);
        r.read(&N_ITER_c);
        for (int i=0; i<N_ITER_c; i++) {
            NID_C_t nid_c;
            r.read(&nid_c);
            NID_Cs.push_back(nid_c);
        }
        r.read(&V_NVSHUNT);
        r.read(&V_NVSTFF);
        r.read(&V_NVONSIGHT);
        r.read(&V_NVLIMSUPERV);
        r.read(&V_NVUNFIT);
        r.read(&V_NVREL);
        r.read(&D_NVROLL);
        r.read(&Q_NVSBTSMPERM);
        r.read(&Q_NVEMRRLS);
        r.read(&Q_NVGUIPERM);
        r.read(&Q_NVSBFBPERM);
        r.read(&Q_NVINHSMICPERM);
        r.read(&V_NVALLOWOVTRP);
        r.read(&V_NVSUPOVTRP);
        r.read(&D_NVOVTRP);
        r.read(&T_NVOVTRP);
        r.read(&D_NVPOTRP);
        r.read(&M_NVCONTACT);
        r.read(&T_NVCONTACT);
        r.read(&M_NVDERUN);
        r.read(&D_NVSTFF);
        r.read(&Q_NVDRIVER_ADHES);
        r.read(&A_NVMAXREDADH1);
        r.read(&A_NVMAXREDADH2);
        r.read(&A_NVMAXREDADH3);
        r.read(&Q_NVLOCACC);
        r.read(&M_NVAVADH);
        r.read(&M_NVEBCL);
        r.read(&Q_NVKINT);
        if (Q_NVKINT == 1) {
            element_kv = KVINT_element(r);
            r.read(&N_ITER_kv);
            for (int i=0; i<N_ITER_kv; i++) {
                elements_kv.push_back(KVINT_element(r));
            }
            element_kr = KRINT_element(r);
            r.read(&N_ITER_kr);
            for (int i=0; i<N_ITER_kr; i++) {
                elements_kr.push_back(KRINT_element(r));
            }
            r.read(&M_NVKTINT);
        }
    }
    NationalValues *create(bit_read_temp &r)
    {
        return new NationalValues(r);
    }
    void serialize(bit_write &w) override
    {
        w.write(&NID_PACKET);
        w.write(&Q_DIR);
        w.write(&L_PACKET);
        w.write(&Q_SCALE);
        w.write(&D_VALIDNV);
        w.write(&NID_C);
        w.write(&N_ITER_c);
        for (int i=0; i<N_ITER_c; i++) {
            w.write(&NID_Cs[i]);
        }
        w.write(&V_NVSHUNT);
        w.write(&V_NVSTFF);
        w.write(&V_NVONSIGHT);
        w.write(&V_NVLIMSUPERV);
        w.write(&V_NVUNFIT);
        w.write(&V_NVREL);
        w.write(&D_NVROLL);
        w.write(&Q_NVSBTSMPERM);
        w.write(&Q_NVEMRRLS);
        w.write(&Q_NVGUIPERM);
        w.write(&Q_NVSBFBPERM);
        w.write(&Q_NVINHSMICPERM);
        w.write(&V_NVALLOWOVTRP);
        w.write(&V_NVSUPOVTRP);
        w.write(&D_NVOVTRP);
        w.write(&T_NVOVTRP);
        w.write(&D_NVPOTRP);
        w.write(&M_NVCONTACT);
        w.write(&T_NVCONTACT);
        w.write(&M_NVDERUN);
        w.write(&D_NVSTFF);
        w.write(&Q_NVDRIVER_ADHES);
        w.write(&A_NVMAXREDADH1);
        w.write(&A_NVMAXREDADH2);
        w.write(&A_NVMAXREDADH3);
        w.write(&Q_NVLOCACC);
        w.write(&M_NVAVADH);
        w.write(&M_NVEBCL);
        w.write(&Q_NVKINT);
        if (Q_NVKINT == 1) {
            element_kv.serialize(w);
            w.write(&N_ITER_kv);
            for (int i=0; i<N_ITER_kv; i++) {
                elements_kv[i].serialize(w);
            }
            element_kr.serialize(w);
            w.write(&N_ITER_kr);
            for (int i=0; i<N_ITER_kr; i++) {
                elements_kr[i].serialize(w);
            }
            w.write(&M_NVKTINT);
        }
    }
};