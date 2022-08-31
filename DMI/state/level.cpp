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
#include "level.h"
#include <string>
using namespace std;
extern Component distanceBar;
Component a4(54, 25, nullptr);
Component levelRegion(54, 25, displayLevel);
static Level prevlevel;
void displayLevel()
{
    if(prevlevel==level) return;
    prevlevel = level;
    levelRegion.clear();
    string path = "symbols/Level/LE_";
    int num = 0;
    switch(level)
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
    if (num == 0) return;
    if(num<10) path+="0";
    path+=to_string(num);
    //if (level == Level::NTC) path += "_" + to_string(nid_ntc);
    path+=".bmp";
    levelRegion.addImage(path);
}