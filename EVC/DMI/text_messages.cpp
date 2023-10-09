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
    auto datetime = offset_time();
    hour = datetime.hour;
    minute = datetime.minute;
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
void add_message(PlainTextMessage m, distance ref)
{
    std::string text = X_TEXT_t::getUTF8(m.X_TEXT);
    std::function<bool(text_message&)> start = [m, ref](text_message &t) {
        bool waitall = (m.Q_TEXTDISPLAY == Q_TEXTDISPLAY_t::WaitAll);
        std::vector<bool> conds;
        if (m.D_TEXTDISPLAY != D_TEXTDISPLAY_t::NotDistanceLimited)
            conds.push_back(d_estfront>ref.est+m.D_TEXTDISPLAY.get_value(m.Q_SCALE));
        if (m.M_MODETEXTDISPLAY1!=M_MODETEXTDISPLAY_t::NoModeLimited)
            conds.push_back(mode == m.M_MODETEXTDISPLAY1.get_value());
        if (m.M_LEVELTEXTDISPLAY1!=M_LEVELTEXTDISPLAY_t::NoLevelLimited)
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
    std::function<bool(text_message&)> end = [m, ref](text_message &t) {
        bool waitall = (m.Q_TEXTDISPLAY == Q_TEXTDISPLAY_t::WaitAll);
        std::vector<bool> conds;
        if (m.L_TEXTDISPLAY != L_TEXTDISPLAY_t::NotDistanceLimited)
            conds.push_back(t.shown && d_estfront>t.first_distance.est+m.L_TEXTDISPLAY.get_value(m.Q_SCALE));
        if (m.M_MODETEXTDISPLAY2!=M_MODETEXTDISPLAY_t::NoModeLimited)
            conds.push_back(mode == m.M_MODETEXTDISPLAY2.get_value());
        if (m.M_LEVELTEXTDISPLAY2!=M_LEVELTEXTDISPLAY_t::NoLevelLimited)
            conds.push_back(level == m.M_LEVELTEXTDISPLAY2.get_value());
        if (m.T_TEXTDISPLAY != T_TEXTDISPLAY_t::NoTimeLimited)
            conds.push_back(t.shown && t.first_displayed+m.T_TEXTDISPLAY*1000<get_milliseconds());
        bool cond = waitall;
        for (int i=0; i<conds.size(); i++) {
            if (waitall)
                cond &= conds[i];
            else
                cond |= conds[i];
        }
        if (m.Q_TEXTCONFIRM != Q_TEXTCONFIRM_t::NoConfirm)
            return (m.Q_CONFTEXTDISPLAY == Q_CONFTEXTDISPLAY_t::AcknowledgeEnds || cond || (operated_version>>4)==1) && t.acknowledged;
        return cond;
    };
    text_message t(text, m.Q_TEXTCLASS == Q_TEXTCLASS_t::ImportantInformation, m.Q_TEXTCONFIRM != Q_TEXTCONFIRM_t::NoConfirm, 0, end);
    t.start_condition = start;
    t.type = text_message_type::PlainText;
    if (m.Q_TEXTREPORT == Q_TEXTREPORT_t::AckReport) {
        t.trackid = m.NID_TEXTMESSAGE;
        t.report_rbc = {m.NID_C, m.NID_RBC, 0};
    }
    add_message(t);
}
void add_message(FixedTextMessage m, distance ref)
{
    std::string text;
    switch (m.Q_TEXT.rawdata) {
        case Q_TEXT_t::LXNotProtected: text = get_text("Level crossing not protected"); break;
        case Q_TEXT_t::Acknowledgement: text = get_text("Acknowledgement"); break;
    }
    std::function<bool(text_message&)> start = [m, ref](text_message &t) {
        bool waitall = (m.Q_TEXTDISPLAY == Q_TEXTDISPLAY_t::WaitAll);
        std::vector<bool> conds;
        if (m.D_TEXTDISPLAY != D_TEXTDISPLAY_t::NotDistanceLimited)
            conds.push_back(d_estfront>ref.est+m.D_TEXTDISPLAY.get_value(m.Q_SCALE));
        if (m.M_MODETEXTDISPLAY1!=M_MODETEXTDISPLAY_t::NoModeLimited)
            conds.push_back(mode == m.M_MODETEXTDISPLAY1.get_value());
        if (m.M_LEVELTEXTDISPLAY1!=M_LEVELTEXTDISPLAY_t::NoLevelLimited)
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
    std::function<bool(text_message&)> end = [m, ref](text_message &t) {
        bool waitall = (m.Q_TEXTDISPLAY == Q_TEXTDISPLAY_t::WaitAll);
        std::vector<bool> conds;
        if (m.L_TEXTDISPLAY != L_TEXTDISPLAY_t::NotDistanceLimited)
            conds.push_back(t.shown && d_estfront>t.first_distance.est+m.L_TEXTDISPLAY.get_value(m.Q_SCALE));
        if (m.M_MODETEXTDISPLAY2!=M_MODETEXTDISPLAY_t::NoModeLimited)
            conds.push_back(mode == m.M_MODETEXTDISPLAY2.get_value());
        if (m.M_LEVELTEXTDISPLAY2!=M_LEVELTEXTDISPLAY_t::NoLevelLimited)
            conds.push_back(level == m.M_LEVELTEXTDISPLAY2.get_value());
        if (m.T_TEXTDISPLAY != T_TEXTDISPLAY_t::NoTimeLimited)
            conds.push_back(t.shown && t.first_displayed+m.T_TEXTDISPLAY*1000<get_milliseconds());
        bool cond = waitall;
        for (int i=0; i<conds.size(); i++) {
            if (waitall)
                cond &= conds[i];
            else
                cond |= conds[i];
        }
        if (m.Q_TEXTCONFIRM != Q_TEXTCONFIRM_t::NoConfirm)
            return (m.Q_CONFTEXTDISPLAY == Q_CONFTEXTDISPLAY_t::AcknowledgeEnds || cond || (operated_version>>4)==1) && t.acknowledged;
        return cond;
    };
    text_message t(text, m.Q_TEXTCLASS == Q_TEXTCLASS_t::ImportantInformation, m.Q_TEXTCONFIRM != Q_TEXTCONFIRM_t::NoConfirm, 0, end);
    t.start_condition = start;
    t.type = text_message_type::FixedText;
    if (m.Q_TEXTREPORT == Q_TEXTREPORT_t::AckReport) {
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
                auto *ack = new text_message_ack_message();
                ack->NID_TEXTMESSAGE.rawdata = msg.trackid;
                fill_message(ack);
                supervising_rbc->send(std::shared_ptr<euroradio_message_traintotrack>(ack));
            }
        }
    }
}