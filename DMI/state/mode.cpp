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
#include "mode.h"
#include "../monitor.h"
#include "../graphics/component.h"
#include <string>
using namespace std;
extern Component csg;
Component modeRegion(36,36, displayMode);
static Mode prevmode;
void displayMode()
{
    if(mode==prevmode) return;
    prevmode = mode;
    modeRegion.clear();
    string path = "symbols/Mode/MO_";
    int num;
    switch(mode)
    {
        case Mode::SH:
            num = 1;
            break;
        case Mode::TR:
            num = 4;
            break;
        case Mode::PT:
            num = 6;
            break;
        case Mode::OS:
            num = 7;
            break;
        case Mode::SR:
            num = 9;
            break;
        case Mode::FS:
            num = 11;
            break;
        case Mode::NL:
            num = 12;
            break;
        case Mode::SB:
            num = 13;
            break;
        case Mode::RV:
            num = 14;
            break;
        case Mode::UN:
            num = 16;
            break;
        case Mode::SF:
            num = 18;
            break;
        case Mode::NS:
            num = 19;
            break;
        case Mode::LS:
            num = 21;
            break;
    }
    if(num<10) path+="0";
    path+=to_string(num);
    path+=".bmp";
    modeRegion.addImage(path);
}