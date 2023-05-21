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
#include "../STM/stm_objects.h"
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
    if (mode == Mode::SN)
    {
        if (active_ntc_window == nullptr || !(active_ntc_window->monitoring_data.Dtarget_display & 1))
        {
            a2.clear();
            prev_dist = -1;
            return;
        }
    }
    else if (mode != Mode::FS && mode != Mode::OS && mode != Mode::SR && mode != Mode::RV)
    {
        a2.clear();
        prev_dist = -1;
        return;
    }
    else if((monitoring == CSM && Vtarget>=Vperm) || (!showSpeeds && (mode == Mode::OS || mode == Mode::SR)))
    {
        a2.clear();
        prev_dist = -1;
        return;
    }
    float dist = Dtarg;
    if(dist!=prev_dist)
    {
        a2.clear();
        a2.addText(to_string(((((int)(dist))/10))*10), 10, 0, 10, Grey, RIGHT);
        prev_dist = dist;
    }
}
void displayDistance()
{
    float dist = Dtarg;
    if (mode == Mode::SN)
    {
        if (active_ntc_window == nullptr || active_ntc_window->monitoring_data.Dtarget_display < 2)
            return;
    }
    else if (mode != Mode::FS && mode != Mode::OS && mode != Mode::SR && mode != Mode::RV) return;
    else if(monitoring == CSM && Vtarget>=Vperm) return;
    else if(!showSpeeds && (mode == Mode::OS || mode == Mode::SR)) return;
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
