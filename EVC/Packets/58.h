#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>
struct PositionReportLocationElement
{
    D_LOC_t D_LOC;
    Q_LGTLOC_t Q_LGTLOC;
    void copy(bit_manipulator &r)
    {
        D_LOC.copy(r);
        Q_LGTLOC.copy(r);
    }
};
struct PositionReportParameters : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    T_CYCLOC_t T_CYCLOC;
    D_CYCLOC_t D_CYCLOC;
    M_LOC_t M_LOC;
    N_ITER_t N_ITER;
    std::vector<PositionReportLocationElement> elements;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_SCALE.copy(r);
        T_CYCLOC.copy(r);
        D_CYCLOC.copy(r);
        M_LOC.copy(r);
        N_ITER.copy(r);
        elements.resize(N_ITER);
        for (int i=0; i<N_ITER; i++) {
            elements[i].copy(r);
        }
    }
};