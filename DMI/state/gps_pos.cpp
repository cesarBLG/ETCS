/*
 * European Train Control System 
 * Copyright (C) 2019  Iván Izquierdo
 * Copyright (C) 2020  César Benito <cesarbema2009@hotmail.com>
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
#include "gps_pos.h"
Component gps_pos(120,50,display_gps);
static bool show_pos = false;
int prevkm=-1;
int prevm=-1;
double pk=-1;
double prevpk=-1;
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
        if (pk < 0) {
            gps_pos.setBackgroundColor(DarkBlue);
            gps_pos.setPressedAction(nullptr);
        } else {
            gps_pos.setPressedAction([]() {show_pos = !show_pos; prevpk = -1;});
            if (!show_pos) {
                gps_pos.addImage("symbols/Driver Request/DR_03.bmp");
                gps_pos.setBackgroundColor(DarkBlue);
            } else {
                string tm = to_string(m/100)+to_string((m/10)%10)+to_string(m%10);
                gps_pos.setBackgroundColor(Grey);
                gps_pos.addText(to_string(km), 62, 0, 12, Black, RIGHT);
                gps_pos.addText(tm, 62, 0, 10, Black, LEFT);
            }
        }
    }
}
