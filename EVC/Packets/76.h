#pragma once
#include "types.h"
#include "variables.h"
#include "packets.h"
#include <vector>
struct FixedTextMessage : ETCS_directional_packet
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
    Q_TEXT_t Q_TEXT;
    FixedTextMessage() = default;
    FixedTextMessage(bit_read_temp &r)
    {
        r.read(&NID_PACKET);
        r.read(&Q_DIR);
        r.read(&L_PACKET);
        r.read(&Q_SCALE);
        r.read(&Q_TEXTCLASS);
        r.read(&Q_TEXTDISPLAY);
        r.read(&D_TEXTDISPLAY);
        r.read(&M_MODETEXTDISPLAY1);
        r.read(&M_LEVELTEXTDISPLAY1);
        if (M_LEVELTEXTDISPLAY1 == 1)
            r.read(&NID_NTC1);
        r.read(&L_TEXTDISPLAY);
        r.read(&T_TEXTDISPLAY);
        r.read(&M_MODETEXTDISPLAY2);
        r.read(&M_LEVELTEXTDISPLAY2);
        if (M_LEVELTEXTDISPLAY2 == 1)
            r.read(&NID_NTC2);
        r.read(&Q_TEXTCONFIRM);
        if (Q_TEXTCONFIRM != 0) {
            r.read(&Q_CONFTEXTDISPLAY);
            r.read(&Q_TEXTREPORT);
            if (Q_TEXTREPORT == 1) {
                r.read(&NID_TEXTMESSAGE);
                r.read(&NID_C);
                r.read(&NID_RBC);
            }
        }
        r.read(&Q_TEXT);
    }
    FixedTextMessage *create(bit_read_temp &r) override
    {
        return new FixedTextMessage(r);
    }
};