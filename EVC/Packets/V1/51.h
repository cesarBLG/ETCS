#pragma once
#include "../types.h"
#include "variables.h"
#include "../packets.h"
#include <vector>
namespace V1
{
struct ASP_diff
{
    M_AXLELOAD_t M_AXLELOAD;
    V_AXLELOAD_t V_AXLELOAD;
    void copy(bit_manipulator &r)
    {
        M_AXLELOAD.copy(r);
        V_AXLELOAD.copy(r);
    }
};
struct ASP_element_packet
{
    D_AXLELOAD_t D_AXLELOAD;
    L_AXLELOAD_t L_AXLELOAD;
    Q_FRONT_t Q_FRONT;
    N_ITER_t N_ITER;
    std::vector<ASP_diff> diffs;
    void copy(bit_manipulator &r)
    {
        D_AXLELOAD.copy(r);
        L_AXLELOAD.copy(r);
        Q_FRONT.copy(r);
        N_ITER.copy(r);
        diffs.resize(N_ITER);
        for (int i=0; i<N_ITER; i++) {
            diffs[i].copy(r);
        }
    }
};
struct AxleLoadSpeedProfile : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    Q_TRACKINIT_t Q_TRACKINIT;
    D_TRACKINIT_t D_TRACKINIT;
    ASP_element_packet element;
    N_ITER_t N_ITER;
    std::vector<ASP_element_packet> elements;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_SCALE.copy(r);
        Q_TRACKINIT.copy(r);
        if (Q_TRACKINIT.rawdata == 0) {
            D_TRACKINIT.copy(r);
        } else {
            element.copy(r);
            N_ITER.copy(r);
            elements.resize(N_ITER);
            for (int i=0; i<N_ITER; i++)
            {
                elements[i].copy(r);
            }
        }
    }
};
}