#pragma once
#include <vector>
#include "types.h"
#include "packets.h"
struct Level1_MA_Section
{
    L_SECTION_t L_SECTION;
    Q_SECTIONTIMER_t Q_SECTIONTIMER;
    T_SECTIONTIMER_t T_SECTIONTIMER; 
    D_SECTIONTIMERSTOPLOC_t D_SECTIONTIMERSTOPLOC;
    void copy(bit_manipulator &r)
    {
        L_SECTION.copy(r);
        Q_SECTIONTIMER.copy(r);
        if (Q_SECTIONTIMER) {
            T_SECTIONTIMER.copy(r);
            D_SECTIONTIMERSTOPLOC.copy(r);
        }
    }
};
struct Level1_MA : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    V_MAIN_t V_MAIN;
    V_EMA_t V_EMA;
    T_EMA_t T_EMA;
    N_ITER_t N_ITER;
    std::vector<Level1_MA_Section> sections;
    L_ENDSECTION_t L_ENDSECTION;
    Q_SECTIONTIMER_t Q_SECTIONTIMER;
    T_SECTIONTIMER_t T_SECTIONTIMER;
    D_SECTIONTIMERSTOPLOC_t D_SECTIONTIMERSTOPLOC;
    Q_ENDTIMER_t Q_ENDTIMER;
    T_ENDTIMER_t T_ENDTIMER;
    D_ENDTIMERSTARTLOC_t D_ENDTIMERSTARTLOC;
    Q_DANGERPOINT_t Q_DANGERPOINT;
    D_DP_t D_DP;
    V_RELEASEDP_t V_RELEASEDP;
    Q_OVERLAP_t Q_OVERLAP;
    D_STARTOL_t D_STARTOL;
    T_OL_t T_OL;
    D_OL_t D_OL;
    V_RELEASEOL_t V_RELEASEOL;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_SCALE.copy(r);
        V_MAIN.copy(r);
        V_EMA.copy(r);
        T_EMA.copy(r);
        N_ITER.copy(r);
        sections.resize(N_ITER);
        for (int i=0; i<N_ITER; i++) {
            sections[i].copy(r);
        }
        L_ENDSECTION.copy(r);
        Q_SECTIONTIMER.copy(r);
        if (Q_SECTIONTIMER) {
            T_SECTIONTIMER.copy(r);
            D_SECTIONTIMERSTOPLOC.copy(r);
        }
        Q_ENDTIMER.copy(r);
        if (Q_ENDTIMER) {
            T_ENDTIMER.copy(r);
            D_ENDTIMERSTARTLOC.copy(r);
        }
        Q_DANGERPOINT.copy(r);
        if (Q_DANGERPOINT) {
            D_DP.copy(r);
            V_RELEASEDP.copy(r);
        }
        Q_OVERLAP.copy(r);
        if (Q_OVERLAP) {
            D_STARTOL.copy(r);
            T_OL.copy(r);
            D_OL.copy(r);
            V_RELEASEOL.copy(r);
        }
    }
};