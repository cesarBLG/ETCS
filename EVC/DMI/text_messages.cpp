/*
 * European Train Control System
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "text_message.h"
#include "../Time/clock.h"
#include "../Supervision/supervision.h"
#include <locale.h>
unsigned char idcount=0;
text_message::text_message(std::string text, bool fg, bool ack, int reason, std::function<bool(text_message&)> end_condition) 
    : text(text), firstGroup(fg), ack(ack), reason(reason), end_condition(end_condition)
{
    std::time_t t = std::time(0);
    std::tm* now = std::localtime(&t);
    hour = now->tm_hour;
    minute = now->tm_min;
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
            it->first_distance = d_estfront_dir[odometer_orientation == -1];
            send(*it);
            it->first_displayed = get_milliseconds();
        }
        ++it;
    }
}
void add_message(PlainTextMessage m, distance ref)
{
    std::string text;
    for (int i=0; i<m.X_TEXT.size(); i++) {
        unsigned char c = m.X_TEXT[i];
        if (m.X_TEXT[i] < 0x80) {
            text += c;
        } else {
            text += 0xc2+(c>0xbf);
            text += (c&0x3f)+0x80;
        }
    }
    std::function<bool(text_message&)> start = [m, ref](text_message &t) {
        bool waitall = (m.Q_TEXTDISPLAY == Q_TEXTDISPLAY_t::WaitAll);
        std::vector<bool> conds;
        if (m.D_TEXTDISPLAY != D_TEXTDISPLAY_t::NotDistanceLimited)
            conds.push_back(d_estfront>ref+m.D_TEXTDISPLAY.get_value(m.Q_SCALE));
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
            conds.push_back(t.shown && d_estfront>t.first_distance+m.L_TEXTDISPLAY.get_value(m.Q_SCALE));
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
            return (m.Q_CONFTEXTDISPLAY == Q_CONFTEXTDISPLAY_t::AcknowledgeEnds || cond) && t.acknowledged;
        return cond;
    };
    text_message t(text, m.Q_TEXTCLASS == Q_TEXTCLASS_t::ImportantInformation, m.Q_TEXTCONFIRM != Q_TEXTCONFIRM_t::NoConfirm, 0, end);
    t.start_condition = start;
    t.type = text_message_type::FixedText;
    add_message(t);
}
void message_acked(int id)
{
    for (auto it = messages.begin(); it!=messages.end(); ++it) {
        if (it->id == id) it->acknowledged = true;
    }
}