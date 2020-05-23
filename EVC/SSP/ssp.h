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
#include "../Position/distance.h"
#include "../Packets/27.h"
#include <map>
#include <set>
#include <vector>
struct SSP_element
{
    distance start;
    bool compensate_train_length;
    std::map<int,double> restrictions[3];
    SSP_element() = default;
    SSP_element(distance start, double basic_speed, bool comp) : start(start), compensate_train_length(comp)
    {
        restrictions[0][-1] = basic_speed;
    }
    double get_speed(int train_cant_deficiency, std::set<int> train_categories)
    {
        double v = (--restrictions[0].upper_bound(train_cant_deficiency))->second;
        double v2 = 10000;
        bool replaces = false;
        for (int cat : train_categories) {
            auto it = restrictions[1].find(cat);
            if (it!=restrictions[1].end()) {
                v2 = std::min(v2, it->second);
                replaces = true;
            }
        }
        if (replaces)
            v = v2;
        for (int cat : train_categories) {
            auto it = restrictions[2].find(cat);
            if (it!=restrictions[2].end()) {
                v = std::min(v, it->second);
            }
        }
        return v;
    }
};
std::vector<SSP_element> get_SSP(distance start, InternationalSSP issp);