#pragma once
#include "../3.h"
#include <vector>
namespace V1
{
struct NationalValuesBraking : ETCS_directional_packet
{
    Q_NVGUIPERM_t Q_NVGUIPERM;
    Q_NVSBFBPERM_t Q_NVSBFBPERM;
    Q_NVINHSMICPERM_t Q_NVINHSMICPERM;
    A_NVMAXREDADH1_t A_NVMAXREDADH1;
    A_NVMAXREDADH2_t A_NVMAXREDADH2;
    A_NVMAXREDADH3_t A_NVMAXREDADH3;
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
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        Q_DIR.copy(w);
        L_PACKET.copy(w);
        Q_NVGUIPERM.copy(w);
        Q_NVSBFBPERM.copy(w);
        Q_NVINHSMICPERM.copy(w);
        A_NVMAXREDADH1.copy(w);
        A_NVMAXREDADH2.copy(w);
        A_NVMAXREDADH3.copy(w);
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
}
