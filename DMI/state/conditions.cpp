/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "../graphics/component.h"
#include <string>
#include <set>
#include "../monitor.h"
using namespace std;
extern Component csg;
int track_conditions[] = {0,0,0};
Component b3(36, 36);
Component b4(36, 36);
Component b5(36, 36);
void updateTc(std::set<int> &syms)
{
    Component* conds[] = {&b3,&b4,&b5};
    std::set<int> asig;
    for (int i=0; i<3; i++) 
    {
        if (track_conditions[i] != 0)
        {
            if (syms.find(track_conditions[i]) == syms.end())
            {
                track_conditions[i] = 0;
                conds[i]->clear();
            }
            else asig.insert(track_conditions[i]);
        }
    }
    for (int s : syms)
    {
        if (asig.size()>=3) break;
        if (asig.find(s) != asig.end()) continue;
        for (int i=0; i<3; i++)
        {
            if (track_conditions[i] == 0)
            {
                track_conditions[i] = s;
                string path = "symbols/Track Conditions/TC_";
                if(s<10) path+= "0";
                path+= to_string(s);
                path+= ".bmp";
                if (s == 100) path = "symbols/Level Crossing/LX_01.bmp";
                conds[i]->addImage(path);
                asig.insert(s);
                break;
            }
        }
    }
}