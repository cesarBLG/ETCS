/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "gps_pos.h"
Component gps_pos;
Button gpsButton(64, 50);
static bool show_pos = true;
int prevkm=-1;
int prevm=-1;
double pk=-1;
double prevpk=-1;
void setupGpsPos()
{
    gps_pos = Component(120,softkeys ? 30 : 50,display_gps);
}
void display_gps()
{
    int m = ((int)pk)%1000;
    int km = (int)(pk/1000);
    bool changed = pk * prevpk < 0 || (show_pos && (prevkm != km || prevm != m));
    prevm = m;
    prevkm = km;
    prevpk = pk;
    if (changed) {
        gps_pos.clear();
        gpsButton.clear();
        if (pk < 0) {
            gps_pos.setBackgroundColor(DarkBlue);
            gps_pos.setPressedAction(nullptr);
            gpsButton.setPressedAction(nullptr);
            gpsButton.setEnabled(false);
        } else {
            auto action = []() {show_pos = !show_pos; prevpk = -1;};
            gps_pos.setPressedAction(action);
            gpsButton.setPressedAction(action);
            gpsButton.setEnabled(true);
            gpsButton.addImage("symbols/Driver Request/DR_03.bmp");
            if (!show_pos) {
                if (!softkeys) gps_pos.addImage("symbols/Driver Request/DR_03.bmp");
                gps_pos.setBackgroundColor(DarkBlue);
            } else {
                std::string tm = std::to_string(m/100)+std::to_string((m/10)%10)+std::to_string(m%10);
                gps_pos.setBackgroundColor(Grey);
                gps_pos.addText(std::to_string(km), 62, 0, 12, Black, RIGHT);
                gps_pos.addText(tm, 62, 0, 10, Black, LEFT);
            }
        }
    }
}
