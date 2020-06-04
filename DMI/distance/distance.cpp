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
#include "distance.h"
#include <string>
using namespace std;
const int posy[] = {-1,6,13,22,32,45,59,79,105,152,185};
const int posx[] = {12,16,16,16,16,12,16,16,16,16,12};
extern Component a1;
Component a2(54,30, displayDistanceText);
Component distanceBar(54,191, displayDistance);
Component a23(54, 221, nullptr);
extern bool showSpeeds;
static float prev_dist = 0;
void displayDistanceText()
{
    if((monitoring == CSM && Vtarget>=Vperm) || (!showSpeeds && (mode == Mode::OS || mode == Mode::SR))) {
        a2.clear();
        return;
    }
    float dist = Dtarg;
    if(dist!=prev_dist)
    {
        a2.clear();
        string str = to_string(((((int)(dist))/10))*10);
        a2.addText(str, 10, 0, 10, Grey, RIGHT);
        prev_dist = dist;
    }
}
void displayDistance()
{
    float dist = Dtarg;
    if(monitoring == CSM && Vtarget>=Vperm) return;
    if(!showSpeeds && (mode == Mode::OS || mode == Mode::SR)) return;
    for(int i=0; i<11; i++)
    {
        int dist = 1000-i*100;
        distanceBar.drawRectangle(posx[i], posy[i], 25-posx[i], 1, Grey);
    }
    if(dist>1000) dist = 1000;
    float h = 0;
    if(dist<100) h = dist/100*(185-152);
    else
    {
        h = 185-152;
        h += (log10(dist)-2)*(152+1);
    }
    distanceBar.drawRectangle(29, 186-h, 10, h, Grey);
    /*for(float y = 186; y>=(186-h); y-=0.5)
    {
        distanceBar.drawLine(29, y, 39, y);
    }*/
}
