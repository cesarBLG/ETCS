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