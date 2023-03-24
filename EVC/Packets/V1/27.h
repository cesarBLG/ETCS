#pragma once
#include "variables.h"
#include "../packets.h"
#include <vector>
namespace V1
{
struct SSP_diff
{
    NC_DIFF_t NC_DIFF;
    V_DIFF_t V_DIFF;
    void copy(bit_manipulator &r)
    {
        NC_DIFF.copy(r);
        V_DIFF.copy(r);
    }
};
struct SSP_element_packet
{
    D_STATIC_t D_STATIC;
    V_STATIC_t V_STATIC;
    Q_FRONT_t Q_FRONT;
    N_ITER_t N_ITER;
    std::vector<SSP_diff> diffs;
    void copy(bit_manipulator &r)
    {
        D_STATIC.copy(r);
        V_STATIC.copy(r);
        Q_FRONT.copy(r);
        N_ITER.copy(r);
        diffs.resize(N_ITER);
        for (int i=0; i<N_ITER; i++) {
            diffs[i].copy(r);
        }
    }
};
struct InternationalSSP : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    SSP_element_packet element;
    N_ITER_t N_ITER;
    std::vector<SSP_element_packet> elements;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_SCALE.copy(r);
        element.copy(r);
        N_ITER.copy(r);
        elements.resize(N_ITER);
        for (int i=0; i<N_ITER; i++)
        {
            elements[i].copy(r);
        }
    }
};
}