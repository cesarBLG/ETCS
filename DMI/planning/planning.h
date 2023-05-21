/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _PLANNING_H
#define _PLANNING_H
#include <vector>
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
        return (e.speed == speed) && (e.distance == distance);
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
extern std::vector<planning_element> planning_elements;
extern std::vector<gradient_element> gradient_elements;
extern std::vector<speed_element> speed_elements;
extern indication_marker imarker;
void displayPlanning();
void displayObjects();
void displayGradient();
void displaySpeed();
void displayPASP();
float getPlanningHeight(float dist);
void drawObjects(int num, int distance);
#endif
