#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>
struct PlainTextMessage : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    Q_TEXTCLASS_t Q_TEXTCLASS;
    Q_TEXTDISPLAY_t Q_TEXTDISPLAY;
    D_TEXTDISPLAY_t D_TEXTDISPLAY;
    M_MODETEXTDISPLAY_t M_MODETEXTDISPLAY1;
    M_LEVELTEXTDISPLAY_t M_LEVELTEXTDISPLAY1;
    NID_NTC_t NID_NTC1;
    L_TEXTDISPLAY_t L_TEXTDISPLAY;
    T_TEXTDISPLAY_t T_TEXTDISPLAY;
    M_MODETEXTDISPLAY_t M_MODETEXTDISPLAY2;
    M_LEVELTEXTDISPLAY_t M_LEVELTEXTDISPLAY2;
    NID_NTC_t NID_NTC2;
    Q_TEXTCONFIRM_t Q_TEXTCONFIRM;
    Q_CONFTEXTDISPLAY_t Q_CONFTEXTDISPLAY;
    Q_TEXTREPORT_t Q_TEXTREPORT;
    NID_TEXTMESSAGE_t NID_TEXTMESSAGE;
    NID_C_t NID_C;
    NID_RBC_t NID_RBC;
    L_TEXT_t L_TEXT;
    std::vector<X_TEXT_t> X_TEXT;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_SCALE.copy(r);
        Q_TEXTCLASS.copy(r);
        Q_TEXTDISPLAY.copy(r);
        D_TEXTDISPLAY.copy(r);
        M_MODETEXTDISPLAY1.copy(r);
        M_LEVELTEXTDISPLAY1.copy(r);
        if (M_LEVELTEXTDISPLAY1 == 1)
            NID_NTC1.copy(r);
        L_TEXTDISPLAY.copy(r);
        T_TEXTDISPLAY.copy(r);
        M_MODETEXTDISPLAY2.copy(r);
        M_LEVELTEXTDISPLAY2.copy(r);
        if (M_LEVELTEXTDISPLAY2 == 1)
            NID_NTC2.copy(r);
        Q_TEXTCONFIRM.copy(r);
        if (Q_TEXTCONFIRM != 0) {
            Q_CONFTEXTDISPLAY.copy(r);
            Q_TEXTREPORT.copy(r);
            if (Q_TEXTREPORT == 1) {
                NID_TEXTMESSAGE.copy(r);
                NID_C.copy(r);
                NID_RBC.copy(r);
            }
        }
        L_TEXT.copy(r);
        X_TEXT.resize(L_TEXT);
        for (int i=0; i<L_TEXT; i++) {
            X_TEXT[i].copy(r);
        }
    }
};