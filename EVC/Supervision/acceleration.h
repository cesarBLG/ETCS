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
#pragma once
//#include <functional>
#include <set>
#include <map>
#include "../Position/distance.h"
struct acceleration
{
    std::map<distance, std::map<double, double>> accelerations;
    std::set<distance> dist_step;
    std::set<double> speed_step;
    acceleration()
    {
        accelerations[distance(std::numeric_limits<double>::lowest(), 0, 0)][0] = 0;
        dist_step.insert(distance(std::numeric_limits<double>::lowest(), 0, 0));
        speed_step.insert(0);
    }
    double operator()(const double V, const distance &d) const
    {
        return (--(--accelerations.upper_bound(d))->second.upper_bound(V))->second;
    }
    friend acceleration operator+(const acceleration &a1, const acceleration &a2);
    
};
acceleration operator+(const acceleration &a1, const acceleration &a2);
