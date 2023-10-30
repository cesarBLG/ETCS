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
#include "level.h"
#include <string>
using namespace std;
extern Component distanceBar;
Component a4(54, 25, nullptr);
Component levelRegion(54, 25, displayLevel);
static Level prevlevel;
static int prevNTC;
void displayLevel()
{
    if(prevlevel==level && (level != Level::NTC || prevNTC == nid_ntc)) return;
    prevlevel = level;
    prevNTC = nid_ntc;
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
#if BASELINE < 4
        case Level::N3:
            num=5;
            break;
#endif
    }
    if (num == 0) return;
    if(num<10) path+="0";
    path+=to_string(num);
    if (level == Level::NTC)
    {
        std::string testpath = path + "_" + to_string(nid_ntc) + ".bmp";
        if (levelRegion.getImageGraphic(testpath) != nullptr) path += "_" + to_string(nid_ntc);
    }
    path+=".bmp";
    levelRegion.addImage(path);
}