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
#include "acceleration.h"
#include <iostream>
#include <chrono>
acceleration operator+(const acceleration a1, const acceleration a2)
{
    acceleration an;
    an.dist_step = a1.dist_step;
    an.speed_step = a1.speed_step;
    an.dist_step.insert(a2.dist_step.begin(), a2.dist_step.end());
    an.speed_step.insert(a2.speed_step.begin(), a2.speed_step.end());

    auto ac1 = a1.accel;
    auto ac2 = a2.accel;
    an.accel = [ac1,ac2](double V, distance d) {return ac1(V,d) + ac2(V,d);};
    return an;
}