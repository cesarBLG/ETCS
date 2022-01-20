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
#include "../monitor.h"
using namespace std;
extern Component csg;
int track_conditions[] = {0,0,0};
void dispTc1();
void dispTc2();
void dispTc3();
Component b3(36, 36, dispTc1);
Component b4(36, 36, dispTc2);
Component b5(36, 36, dispTc3);
void dispTc(int num, Component &b)
{
    /*if(num == 1 && level == NTC)
    {
        b.setText(to_string((int)Vtarget).c_str(), 15, Red);
        return;
    }*/
    if(track_conditions[num]==0) return;
    string path = "symbols/Track Conditions/TC_";
    if(track_conditions[num]<10) path+= "0";
    path+= to_string(track_conditions[num]);
    path+= ".bmp";
    if (track_conditions[num] == 100) path = "symbols/Level Crossing/LX_01.bmp";
    b.drawImage(path.c_str());
}
void dispTc1(){dispTc(0, b3);}
void dispTc2(){dispTc(1, b4);}
void dispTc3(){dispTc(2, b5);}
void addTc(int newtc)
{
    for(int i=0; i<3; i++)
    {
        if(track_conditions[i]==0)
        {
            track_conditions[i] = newtc;
            break;
        }
    }
}