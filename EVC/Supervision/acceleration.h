/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
