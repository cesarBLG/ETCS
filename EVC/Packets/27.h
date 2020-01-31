#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>
struct SSP_diff
{
    Q_DIFF_t Q_DIFF;
    NC_CDDIFF_t NC_CDDIFF;
    NC_DIFF_t NC_DIFF;
    V_DIFF_t V_DIFF;
    SSP_diff() = default;
    SSP_diff(bit_read_temp &r)
    {
        r.read(&Q_DIFF);
        if (Q_DIFF == 0)
            r.read(&NC_CDDIFF);
        else
            r.read(&NC_DIFF);
        r.read(&V_DIFF);
    }
};
struct SSP_element_packet
{
    D_STATIC_t D_STATIC;
    V_STATIC_t V_STATIC;
    Q_FRONT_t Q_FRONT;
    N_ITER_t N_ITER;
    std::vector<SSP_diff> diffs;
    SSP_element_packet() = default;
    SSP_element_packet(bit_read_temp &r)
    {
        r.read(&D_STATIC);
        r.read(&V_STATIC);
        r.read(&Q_FRONT);
        r.read(&N_ITER);
        for (int i=0; i<N_ITER; i++) {
            diffs.push_back(SSP_diff(r));
        }
    }
};
struct InternationalSSP : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    SSP_element_packet element;
    N_ITER_t N_ITER;
    std::vector<SSP_element_packet> elements;
    InternationalSSP() = default;
    InternationalSSP(bit_read_temp &r)
    {
        r.read(&NID_PACKET);
        r.read(&Q_DIR);
        r.read(&L_PACKET);
        r.read(&Q_SCALE);
        element = SSP_element_packet(r);
        r.read(&N_ITER);
        for (int i=0; i<N_ITER; i++)
        {
            elements.push_back(SSP_element_packet(r));
        }
    }
    InternationalSSP *create(bit_read_temp &r) override
    {
        return new InternationalSSP(r);
    }
};