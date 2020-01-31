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
    Level1_MA_Section() = default;
    Level1_MA_Section(bit_read_temp &r)
    {
        r.read(&L_SECTION);
        r.read(&Q_SECTIONTIMER);
        if (Q_SECTIONTIMER) {
            r.read(&T_SECTIONTIMER);
            r.read(&D_SECTIONTIMERSTOPLOC);
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
    Level1_MA() {}
    Level1_MA(bit_read_temp &r)
    {
        r.read(&NID_PACKET);
        r.read(&Q_DIR);
        r.read(&L_PACKET);
        r.read(&Q_SCALE);
        r.read(&V_MAIN);
        r.read(&V_EMA);
        r.read(&T_EMA);
        r.read(&N_ITER);
        for (int i=0; i<N_ITER; i++) {
            sections.push_back(Level1_MA_Section(r));
        }
        r.read(&L_ENDSECTION);
        r.read(&Q_SECTIONTIMER);
        if (Q_SECTIONTIMER) {
            r.read(&T_SECTIONTIMER);
            r.read(&D_SECTIONTIMERSTOPLOC);
        }
        r.read(&Q_ENDTIMER);
        if (Q_ENDTIMER) {
            r.read(&T_ENDTIMER);
            r.read(&D_ENDTIMERSTARTLOC);
        }
        r.read(&Q_DANGERPOINT);
        if (Q_DANGERPOINT) {
            r.read(&D_DP);
            r.read(&V_RELEASEDP);
        }
        r.read(&Q_OVERLAP);
        if (Q_OVERLAP) {
            r.read(&D_STARTOL);
            r.read(&T_OL);
            r.read(&D_OL);
            r.read(&V_RELEASEOL);
        }
    }
    Level1_MA *create(bit_read_temp &r) override
    {
        return new Level1_MA(r);
    }
};