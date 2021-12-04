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
#include "../graphics/component.h"
#include "../monitor.h"
#include "../tcp/server.h"
#include "../sound/sound.h"
using namespace std;
void dispAcks();
Component c234(37*3, 50, nullptr);
Component c2(37, 50, nullptr);
Component c3(37, 50, nullptr);
Component c4(37, 50, nullptr);
Component c1(58, 50, dispAcks);
Component c5(37, 50, nullptr);
Component c6(37, 50, nullptr);
bool prevAck = false;
int prevlevel = 0;
void dispAcks()
{
    if (modeAck == prevAck && prevlevel == levelAck) return;
    if (modeAck || levelAck == 2) playSinfo();
    prevAck = modeAck;
    prevlevel = levelAck;
    c1.clear();
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
            case Mode::NS:
                num = 20;
                break;
            case Mode::LS:
                num = 22;
                break;
        }
        if(num<10) path+="0";
        path+=to_string(num);
        path+=".bmp";
        c1.setAck([](){write_command("modeAcked","");});
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
        //If NTC is LZB/PZB, path+="a";
        path+=".bmp";
        c1.addImage(path.c_str());
        if(levelAck == 2) c1.setAck([](){write_command("levelAcked","");});
        else c1.setAck(nullptr);
    }
    else c1.setAck(nullptr);
}
