/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "../graphics/component.h"
#include "../monitor.h"
#include "../tcp/server.h"
#include "../sound/sound.h"
#include "../messages/messages.h"
#include "acks.h"
#include <list>
using namespace std;
void dispAcks();
Component c234(37*3, 50, nullptr);
Component c2(37, 50, nullptr);
Component c3(37, 50, nullptr);
Component c4(37, 50, nullptr);
Component c1(58, 50, dispAcks);
Component c5(37, 50, nullptr);
Component c6(37, 50, nullptr);
extern Component c9;
extern Component textArea;
bool prevAck = false;
int prevlevel = 0;
bool level_announce = false;
AckType AllowedAck = AckType::None;
Component *componentAck;
list<pair<AckType, int>> pendingAcks;
void dispAcks()
{
    if (modeAck == prevAck && prevlevel == levelAck) return;
    prevAck = modeAck;
    prevlevel = levelAck;
    c1.clear();
    c1.delayType = false;
    if(modeAck)
    {
        string path = "symbols/Mode/MO_";
        int num;
        switch(ackMode)
        {
            case Mode::SH:
                num = 2;
                break;
            case Mode::TR:
                num = 5;
                break;
            case Mode::OS:
                num = 8;
                break;
            case Mode::SR:
                num = 10;
                break;
            case Mode::RV:
                num = 15;
                break;
            case Mode::UN:
                num = 17;
                break;
            case Mode::SN:
                num = 20;
                break;
            case Mode::LS:
                num = 22;
                break;
        }
        if(num<10) path+="0";
        c1.delayType = num == 10;
        path+=to_string(num);
        path+=".bmp";
        c1.setAck([](){write_command("json",R"({"DriverSelection":"ModeAcknowledge"})");});
        c1.addImage(path.c_str());
    }
    else if(levelAck>0)
    {
        string path = "symbols/Level/LE_";
        int num = 0;
        switch(ackLevel)
        {
            case Level::N0:
                num=1;
                break;
            case Level::NTC:
                num=2;
                break;
            case Level::N1:
                num=3;
                break;
            case Level::N2:
                num=4;
                break;
            case Level::N3:
                num=5;
                break;
        }
        num = 4 + 2*num;
        if(levelAck == 2) num++;
        if(num<10) path+="0";
        path+=to_string(num);
        if (ackLevel == Level::NTC && (ackNTC == 0 || ackNTC == 10 || ackNTC == 26)) path += "_" + to_string(ackNTC);
        path+=".bmp";
        c1.addImage(path.c_str());
        if(levelAck == 2) c1.setAck([](){write_command("json",R"({"DriverSelection":"LevelAcknowledge"})");});
        else c1.setAck(nullptr);
    }
    else c1.setAck(nullptr);
}
int64_t get_milliseconds()
{
    return platform->get_timer();
}
int64_t lastAck;
void updateAcks()
{
    if (AllowedAck == AckType::None && lastAck + 1000 < get_milliseconds() && !pendingAcks.empty())
    {
        AckType type = pendingAcks.front().first;
        switch (type)
        {
            case AckType::Message:
                componentAck = &textArea;
                break;
            case AckType::Mode:
                playSinfo();
                modeAck = true;
                componentAck = &c1;
                break;
            case AckType::Level:
                playSinfo();
                levelAck = 2;
                componentAck = &c1;
                break;
            case AckType::Brake:
                playSinfo();
                brakeAck = true;
                componentAck = &c9;
                break;
        }
        AllowedAck = type;
        if (AllowedAck == AckType::Message) updateMessages();
    }
    if (level_announce)
    {
        if (AllowedAck != AckType::Mode && AllowedAck != AckType::Level)
        {
            if (levelAck == 0) playSinfo();
            levelAck = 1;
        }
    }
    else if (levelAck == 1) levelAck = 0;
    if (AllowedAck == AckType::None) componentAck = nullptr;
}
void setAck(AckType type, int id, bool ack)
{
    if (ack)
    {
        if (type == AckType::Level && id == 1)
        {
            level_announce = true;
            setAck(AckType::Level, 2, false);
            return;
        }
        for (auto it = pendingAcks.begin(); it != pendingAcks.end(); ++it)
        {
            if (it->first == type && it->second == id) return;
        }
        pendingAcks.push_back({type, id});
    }
    else
    {
        if (type == AckType::Level && id == 0) level_announce = false;
        for (auto it = pendingAcks.begin(); it != pendingAcks.end(); )
        {
            if (it->first == type && (type != AckType::Message || it->second == id))
            {
                if (it == pendingAcks.begin())
                {
                    if (AllowedAck == AckType::Mode) modeAck = false;
                    else if (AllowedAck == AckType::Brake) brakeAck = false;
                    else if (AllowedAck == AckType::Level)
                    {
                        levelAck = level_announce ? 1 : 0;
                    }
                    lastAck = get_milliseconds();
                    AllowedAck = AckType::None;
                }
                it = pendingAcks.erase(it);
            }
            else ++it;
        }
    }
}