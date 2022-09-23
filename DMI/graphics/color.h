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
#ifndef _COLOR_H
#define _COLOR_H
struct Color
{
    int R;
    int G;
    int B;
    bool operator==(Color b)
    {
        return R == b.R && G==b.G && B==b.B;
    }
    bool operator!=(Color b)
    {
        return !(*this==b);
    }
};
const Color White = {255,255,255};
const Color Black = {0,0,0};
const Color Grey = {195,195,195};
const Color MediumGrey = {150,150,150};
const Color DarkGrey = {85,85,85};
const Color Yellow = {223,223,0};
const Color Shadow = {8, 24, 57};
const Color Red = {191,0,2};
const Color Orange = {234,145,0};
const Color DarkBlue = {3, 17, 34};
const Color PASPdark = {33, 49, 74};
const Color PASPlight = {41, 74, 107};
const Color Blue = {0, 0, 234};
const Color Green = {0, 234, 0};
const Color LightRed = {255, 96, 96};
const Color LightGreen = {96, 255, 96};
#endif