/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "text_message.h"
#include "../Time/clock.h"
#include "../Supervision/supervision.h"
#include "../language/language.h"
#include "../Version/version.h"

unsigned char idcount=0;
text_message::text_message(std::string text, bool fg, bool ack, int reason, std::function<bool(text_message&)> end_condition) 
    : text(text), firstGroup(fg), ack(ack), reason(reason), end_condition(end_condition)
{
    hour = WallClockTime::hour;
    minute = WallClockTime::minute;
    start_condition = [](text_message &t){return true;};
    id = idcount++;
    acknowledged = false;
    shown = false;
}
std::list<text_message> messages;
text_message &add_message(text_message t)
{
    /*for (auto it = messages.begin(); it!=messages.end(); ++it) {
        if (it->id == t.id) {
            *it = t;
            return *it;
        }
    }*/
    messages.push_back(t);
    return messages.back();
}
extern bool sendtoor;
void send(text_message t) {
    sendtoor=true;
    send_command("setMessage", std::to_string(t.id)+","+std::to_string(t.text.size())+","+t.text+","+std::to_string(t.hour)+","+std::to_string(t.minute)+","+(t.firstGroup?"true,":"false,")+(t.ack?"true,":"false,")+std::to_string(t.reason));
}
void update_messages()
{
    for (auto it = messages.begin(); it!=messages.end();) {
        if (it->end_condition(*it)) {
            sendtoor=true;
            send_command("setRevokeMessage", std::to_string(it->id));
            it = messages.erase(it);
            continue;
        } else if (!it->shown && it->start_condition(*it)) {
            it->shown = true;
            it->first_distance = distance::from_odometer(d_estfront_dir[odometer_orientation == -1]);
            send(*it);
            it->first_displayed = get_milliseconds();
        }
        ++it;
    }
}
void add_message(PlainTextMessage m, optional<distance> ref)
{
    std::string text = X_TEXT_t::getUTF8(m.X_TEXT);
    std::function<bool(text_message&)> start = [m, ref](text_message &t) {
        bool waitall = (m.Q_TEXTDISPLAY == m.Q_TEXTDISPLAY.WaitAll);
        std::vector<bool> conds;
        if (m.D_TEXTDISPLAY != m.D_TEXTDISPLAY.NotDistanceLimited)
            conds.push_back(d_estfront>ref->est+m.D_TEXTDISPLAY.get_value(m.Q_SCALE));
        if (m.M_MODETEXTDISPLAY1 != m.M_MODETEXTDISPLAY1.NoModeLimited)
            conds.push_back(mode == m.M_MODETEXTDISPLAY1.get_value());
        if (m.M_LEVELTEXTDISPLAY1 != m.M_LEVELTEXTDISPLAY1.NoLevelLimited)
            conds.push_back(level == m.M_LEVELTEXTDISPLAY1.get_value());
        bool cond = waitall;
        for (int i=0; i<conds.size(); i++) {
            if (waitall)
                cond &= conds[i];
            else
                cond |= conds[i];
        }
        return cond;
    };
    std::function<bool(text_message&)> end = [m](text_message &t) {
        bool waitall = (m.Q_TEXTDISPLAY == m.Q_TEXTDISPLAY.WaitAll);
        std::vector<bool> conds;
        if (m.L_TEXTDISPLAY != m.L_TEXTDISPLAY.NotDistanceLimited)
            conds.push_back(t.shown && d_estfront>t.first_distance.est+m.L_TEXTDISPLAY.get_value(m.Q_SCALE));
        if (m.M_MODETEXTDISPLAY2 != m.M_MODETEXTDISPLAY2.NoModeLimited)
            conds.push_back(mode == m.M_MODETEXTDISPLAY2.get_value());
        if (m.M_LEVELTEXTDISPLAY2 != m.M_LEVELTEXTDISPLAY2.NoLevelLimited)
            conds.push_back(level == m.M_LEVELTEXTDISPLAY2.get_value());
        if (m.T_TEXTDISPLAY != m.T_TEXTDISPLAY.NoTimeLimited)
            conds.push_back(t.shown && t.first_displayed+m.T_TEXTDISPLAY*1000<get_milliseconds());
        bool cond = waitall;
        for (int i=0; i<conds.size(); i++) {
            if (waitall)
                cond &= conds[i];
            else
                cond |= conds[i];
        }
        if (m.Q_TEXTCONFIRM != m.Q_TEXTCONFIRM.NoConfirm)
            return (m.Q_CONFTEXTDISPLAY == m.Q_CONFTEXTDISPLAY.AcknowledgeEnds || cond || (operated_version>>4)==1) && t.acknowledged;
        return cond;
    };
    text_message t(text, m.Q_TEXTCLASS == m.Q_TEXTCLASS.ImportantInformation, m.Q_TEXTCONFIRM != m.Q_TEXTCONFIRM.NoConfirm, 0, end);
    t.start_condition = start;
    t.type = text_message_type::PlainText;
    if (m.Q_TEXTREPORT == m.Q_TEXTREPORT.AckReport) {
        t.trackid = m.NID_TEXTMESSAGE;
        t.report_rbc = {m.NID_C, m.NID_RBC, 0};
    }
    add_message(t);
}
void add_message(FixedTextMessage m, optional<distance> ref)
{
    std::string text;
    if (m.Q_TEXT.rawdata == m.Q_TEXT.LXNotProtected) text = get_text("Level crossing not protected");
    else if (m.Q_TEXT.rawdata == m.Q_TEXT.Acknowledgement) text = get_text("Acknowledgement");
    std::function<bool(text_message&)> start = [m, ref](text_message &t) {
        bool waitall = (m.Q_TEXTDISPLAY == m.Q_TEXTDISPLAY.WaitAll);
        std::vector<bool> conds;
        if (m.D_TEXTDISPLAY != m.D_TEXTDISPLAY.NotDistanceLimited)
            conds.push_back(d_estfront>ref->est+m.D_TEXTDISPLAY.get_value(m.Q_SCALE));
        if (m.M_MODETEXTDISPLAY1 != m.M_MODETEXTDISPLAY1.NoModeLimited)
            conds.push_back(mode == m.M_MODETEXTDISPLAY1.get_value());
        if (m.M_LEVELTEXTDISPLAY1 != m.M_LEVELTEXTDISPLAY1.NoLevelLimited)
            conds.push_back(level == m.M_LEVELTEXTDISPLAY1.get_value());
        bool cond = waitall;
        for (int i=0; i<conds.size(); i++) {
            if (waitall)
                cond &= conds[i];
            else
                cond |= conds[i];
        }
        return cond;
    };
    std::function<bool(text_message&)> end = [m](text_message &t) {
        bool waitall = (m.Q_TEXTDISPLAY == m.Q_TEXTDISPLAY.WaitAll);
        std::vector<bool> conds;
        if (m.L_TEXTDISPLAY != m.L_TEXTDISPLAY.NotDistanceLimited)
            conds.push_back(t.shown && d_estfront>t.first_distance.est+m.L_TEXTDISPLAY.get_value(m.Q_SCALE));
        if (m.M_MODETEXTDISPLAY2 != m.M_MODETEXTDISPLAY2.NoModeLimited)
            conds.push_back(mode == m.M_MODETEXTDISPLAY2.get_value());
        if (m.M_LEVELTEXTDISPLAY2 != m.M_LEVELTEXTDISPLAY2.NoLevelLimited)
            conds.push_back(level == m.M_LEVELTEXTDISPLAY2.get_value());
        if (m.T_TEXTDISPLAY != m.T_TEXTDISPLAY.NoTimeLimited)
            conds.push_back(t.shown && t.first_displayed+m.T_TEXTDISPLAY*1000<get_milliseconds());
        bool cond = waitall;
        for (int i=0; i<conds.size(); i++) {
            if (waitall)
                cond &= conds[i];
            else
                cond |= conds[i];
        }
        if (m.Q_TEXTCONFIRM != m.Q_TEXTCONFIRM.NoConfirm)
            return (m.Q_CONFTEXTDISPLAY == m.Q_CONFTEXTDISPLAY.AcknowledgeEnds || cond || (operated_version>>4)==1) && t.acknowledged;
        return cond;
    };
    text_message t(text, m.Q_TEXTCLASS == m.Q_TEXTCLASS.ImportantInformation, m.Q_TEXTCONFIRM != m.Q_TEXTCONFIRM.NoConfirm, 0, end);
    t.start_condition = start;
    t.type = text_message_type::FixedText;
    if (m.Q_TEXTREPORT == m.Q_TEXTREPORT.AckReport) {
        t.trackid = m.NID_TEXTMESSAGE;
        t.report_rbc = {m.NID_C, m.NID_RBC, 0};
    }
    add_message(t);
}
void message_acked(int id)
{
    for (auto &msg : messages) {
        if (msg.id == id) {
            msg.acknowledged = true;
            if (msg.report_rbc && supervising_rbc && supervising_rbc->status == session_status::Established &&
            supervising_rbc->contact.country == msg.report_rbc->country && supervising_rbc->contact.id == msg.report_rbc->id) {
                auto ack = std::make_shared<text_message_ack_message>();
                ack->NID_TEXTMESSAGE.rawdata = msg.trackid;
                supervising_rbc->queue(ack);
            }
        }
    }
}