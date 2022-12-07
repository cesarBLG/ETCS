#pragma once
#include "../packets.h"
#include "stm_variables.h"
#include <vector>
struct STMSupervisionInformation : ETCS_packet
{
    Q_SCALE_t Q_SCALE;
    V_PERMIT_t V_PERMIT;
    V_TARGET_t V_TARGET;
    V_RELEASE_t V_RELEASE;
    V_INTERV_t V_INTERV;
    D_TARGET_t D_TARGET;
    M_COLOUR_SP_t M_COLOUR_SP;
    M_COLOUR_PS_t M_COLOUR_PS;
    Q_DISPLAY_PS_t Q_DISPLAY_PS;
    M_COLOUR_TS_t M_COLOUR_TS;
    Q_DISPLAY_TS_t Q_DISPLAY_TS;
    M_COLOUR_RS_t M_COLOUR_RS;
    Q_DISPLAY_RS_t Q_DISPLAY_RS;
    M_COLOUR_IS_t M_COLOUR_IS;
    Q_DISPLAY_IS_t Q_DISPLAY_IS;
    Q_DISPLAY_TD_t Q_DISPLAY_TD;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        Q_SCALE.copy(w);
        V_PERMIT.copy(w);
        V_TARGET.copy(w);
        V_RELEASE.copy(w);
        V_INTERV.copy(w);
        D_TARGET.copy(w);
        M_COLOUR_SP.copy(w);
        M_COLOUR_PS.copy(w);
        Q_DISPLAY_PS.copy(w);
        M_COLOUR_TS.copy(w);
        Q_DISPLAY_TS.copy(w);
        M_COLOUR_RS.copy(w);
        Q_DISPLAY_RS.copy(w);
        M_COLOUR_IS.copy(w);
        Q_DISPLAY_IS.copy(w);
        Q_DISPLAY_TD.copy(w);
    }
};
