#pragma once
#include "../72.h"
#include <vector>
namespace V1
{
struct PlainTextMessage : ::PlainTextMessage
{
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
        L_TEXT.copy(r);
        X_TEXT.resize(L_TEXT);
        for (int i=0; i<L_TEXT; i++) {
            X_TEXT[i].copy(r);
        }
    }
};
}