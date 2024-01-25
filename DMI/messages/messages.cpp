/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "messages.h"
#include "../graphics/color.h"
#include "../graphics/component.h"
#include "../graphics/flash.h"
#include "../sound/sound.h"
#include "../graphics/icon_button.h"
#include "../tcp/server.h"
#include "../state/acks.h"
#include "../language/language.h"
#include "platform_runtime.h"
#include "text_strings.h"
#include <string>
#include <deque>
#include <vector>
#include <algorithm>
using namespace std;
Component e2(54, 25, nullptr);
Component e3;
Component e4(54, 25, nullptr);
int nlines;
Component textArea;
void displayArrows();
void arrowUp();
void arrowDown();
IconButton upArrow;
IconButton downArrow;
IconButton softUpArrow("symbols/Navigation/NA_13.bmp", 40, 64, arrowUp, "symbols/Navigation/NA_15.bmp");
IconButton softDownArrow("symbols/Navigation/NA_14.bmp", 40, 64, arrowDown, "symbols/Navigation/NA_16.bmp");
std::deque<Message> messageList;
int line;
int current=0;
void setupMessages()
{
    e3 = Component(54, softkeys ? 30 : 25, nullptr);
    nlines = softkeys ? 4 : 5;
    textArea = Component(234, 20*nlines, displayMessages);
    upArrow = IconButton("symbols/Navigation/NA_13.bmp", 46, softkeys ? 40 : 50, arrowUp, "symbols/Navigation/NA_15.bmp");
    downArrow = IconButton("symbols/Navigation/NA_14.bmp", 46, softkeys ? 40 : 50, arrowDown, "symbols/Navigation/NA_16.bmp");
}
void addMsg(Message m)
{
    bool found = false;
    for (auto it = messageList.begin(); it != messageList.end(); ++it) {
        if (it->Id == m.Id) {
            if (it->ack != m.ack) setAck(AckType::Message, m.Id, m.ack);
            *it = m;
            found = true;
        }
    }
    current = 0;
    if (!found) {
        if (m.ack) setAck(AckType::Message, m.Id, true);
        messageList.push_back(m);
    }
    updateMessages();
}
void revokeMessage(unsigned int id)
{
    current = 0;
    for(int i=0; i<messageList.size(); i++)
    {
        if(messageList[i].Id == id) messageList.erase(messageList.begin() + i);
    }
    setAck(AckType::Message, id, false);
    updateMessages();
}
void revokeMessages()
{
    current = 0;
    for(auto &msg : messageList)
    {
        setAck(AckType::Message, msg.Id, false);
    }
    messageList.clear();
    updateMessages();
}
bool operator < (Message a, Message b)
{
    if(a.firstGroup && !b.firstGroup) return true;
    if(!a.firstGroup && b.firstGroup) return false;
    if(a.hour != b.hour) return a.hour>b.hour;
    if (a.minute != b.minute) return a.minute>b.minute;
    return a.Id > b.Id;
}
void displayMessages()
{
    upArrow.enabled = current>0;
    downArrow.enabled = line>nlines+current;
    softUpArrow.enabled = upArrow.enabled;
    softDownArrow.enabled = downArrow.enabled;
}
void updateMessages()
{
    sort(messageList.begin(), messageList.end());
    vector<Message*> displayMsg;
    bool ack = false;
    for(int i=0; i<messageList.size(); i++)
    {
        if(messageList[i].ack)
        {
            displayMsg.push_back(&messageList[i]);
            ack = true;
            break;
        }
    }
    if(!ack)
    {
        for(int i=0; i<messageList.size(); i++)
        {
            displayMsg.push_back(&messageList[i]);
        }
    }
    if(ack && AllowedAck == AckType::Message)
    {
        Message *msg = displayMsg[0];
        textArea.setAck([msg]() {
            msg->ack = false;
            setAck(AckType::Message, msg->Id, false);
            write_command("json", R"({"DriverSelection":"MessageAcknowledge","MessageId":)"+std::to_string(msg->Id)+"}");
            updateMessages();
        });
    }
    else textArea.setAck(nullptr);
    textArea.clear();
    line = 0;
    auto font_clock = platform->load_font(10, 0, get_language());
    auto font_msg = platform->load_font(12, 0, get_language());
    auto clock_size = font_clock->calc_size("88:88:_");
    for(int i=0; i<displayMsg.size(); i++)
    {
        Message &m = *displayMsg[i];
        std::string date = std::to_string(m.hour) + ":"+ (m.minute<10 ? "0" : "") + std::to_string(m.minute);
        std::string text = m.text;
        for (;;)
        {
            size_t wrap = font_msg->calc_wrap_point(text, 230.0f - clock_size.first);
            int last = text.find_last_of(' ', wrap);
            if (last == string::npos)
                last = wrap;
            if(line<nlines+current && line>=current)
            {
                if (m.bgColor != DarkBlue) textArea.addRectangle(2, (line-current)*20, 234, 20, m.bgColor);
                if (text.size() == m.text.size())
                {
                    if(!m.shown && (m.firstGroup || m.ack)) playSinfo();
                    m.shown = true;
                    textArea.addText(date, 2, 4 + (line-current)*20, 10, m.fgColor, UP | LEFT, m.firstGroup);
                }
                textArea.addText(text.substr(0, last), 2 + clock_size.first, 2 + (line-current)*20, 12, m.fgColor, UP | LEFT, m.firstGroup);
            }
            ++line;
            if (last + 1 >= text.size()) break;
            text = text.substr(last+1);
        }
    }
}
void arrowUp()
{
    if(current==0) return;
    current--;
    updateMessages();
}
void arrowDown()
{
    if(line<=nlines+current) return;
    current++;
    if(current>line-nlines) current = line-nlines;
    updateMessages();
}
