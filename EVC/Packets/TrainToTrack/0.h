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

    PositionReport() = default;
    PositionReport(bit_read_temp &r)
    {
        r.read(&NID_PACKET);
        r.read(&L_PACKET);
        r.read(&Q_SCALE);
        r.read(&NID_LRBG);
        r.read(&L_DOUBTOVER);
        r.read(&L_DOUBTUNDER);
        r.read(&Q_LENGTH);
        if (Q_LENGTH == 1 || Q_LENGTH == 2) {
            r.read(&L_TRAININT);
        }
        r.read(&V_TRAIN);
        r.read(&Q_DIRTRAIN);
        r.read(&M_MODE);
        r.read(&M_LEVEL);
        if (M_LEVEL == 1) {
            r.read(&NID_NTC);
        }
    }
    PositionReport *create(bit_read_temp &r) override
    {
        return new PositionReport(r);
    }
    void serialize(bit_write &w) override
    {
        w.write(&NID_PACKET);
        w.write(&L_PACKET);
        w.write(&Q_SCALE);
        w.write(&NID_LRBG);
        w.write(&L_DOUBTOVER);
        w.write(&L_DOUBTUNDER);
        w.write(&Q_LENGTH);
        if (Q_LENGTH == 1 || Q_LENGTH == 2) {
            w.write(&L_TRAININT);
        }
        w.write(&V_TRAIN);
        w.write(&Q_DIRTRAIN);
        w.write(&M_MODE);
        w.write(&M_LEVEL);
        if (M_LEVEL == 1) {
            w.write(&NID_NTC);
        }
    }
};