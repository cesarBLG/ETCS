#pragma once
#include "../types.h"
#include "../variables.h"
#include "../packets.h"
#include <vector>

struct PositionReport : ETCS_packet
{
    Q_SCALE_t Q_SCALE;
    NID_LRBG_t NID_LRBG;
    D_LRBG_t D_LRBG;
    Q_DIRLRBG_t Q_DIRLRBG;
    Q_DLRBG_t Q_DLRBG;
    L_DOUBTOVER_t L_DOUBTOVER;
    L_DOUBTUNDER_t L_DOUBTUNDER;
    Q_LENGTH_t Q_LENGTH;
    L_TRAININT_t L_TRAININT;
    V_TRAIN_t V_TRAIN;
    Q_DIRTRAIN_t Q_DIRTRAIN;
    M_MODE_t M_MODE;
    M_LEVEL_t M_LEVEL;
    NID_NTC_t NID_NTC;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        Q_SCALE.copy(w);
        NID_LRBG.copy(w);
        D_LRBG.copy(w);
        Q_DIRLRBG.copy(w);
        Q_DLRBG.copy(w);
        L_DOUBTOVER.copy(w);
        L_DOUBTUNDER.copy(w);
        Q_LENGTH.copy(w);
        if (Q_LENGTH == 1 || Q_LENGTH == 2) {
            L_TRAININT.copy(w);
        }
        V_TRAIN.copy(w);
        Q_DIRTRAIN.copy(w);
        M_MODE.copy(w);
        M_LEVEL.copy(w);
        if (M_LEVEL == 1) {
            NID_NTC.copy(w);
        }
    }
};