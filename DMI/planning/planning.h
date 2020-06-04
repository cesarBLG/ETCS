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
#ifndef _PLANNING_H
#define _PLANNING_H
#include <vector>
using namespace std;
struct planning_element
{
    int condition;
    float distance;
};
struct gradient_element
{
    int val;
    float distance;
};
struct speed_element
{
    int speed;
    float distance;
    bool operator==(speed_element e)
    {
        return (e.speed == speed) && (e.distance = distance);
    }
    bool operator!=(speed_element e)
    {
        return !(e == *this);
    }
};
struct indication_marker
{
    float start_distance;
    speed_element element;
};
extern vector<planning_element> planning_elements;
extern vector<gradient_element> gradient_elements;
extern vector<speed_element> speed_elements;
extern indication_marker imarker;
void displayPlanning();
void displayObjects();
void displayGradient();
void displaySpeed();
void displayPASP();
float getPlanningHeight(float dist);
void drawObjects(int num, int distance);
#endif
