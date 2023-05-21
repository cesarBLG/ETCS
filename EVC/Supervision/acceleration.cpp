/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "acceleration.h"
#include <iostream>
#include <chrono>
acceleration operator+(const acceleration &a1, const acceleration &a2)
{
    acceleration an = acceleration();
    an.dist_step = a1.dist_step;
    an.speed_step = a1.speed_step;
    an.dist_step.insert(a2.dist_step.begin(), a2.dist_step.end());
    an.speed_step.insert(a2.speed_step.begin(), a2.speed_step.end());
    for (auto &d : an.dist_step) {
        for (auto &V : an.speed_step) {
            an.accelerations[d][V] = a1(V, d) + a2(V, d);
        }
    }
    /*auto ac1 = a1.accel;
    auto ac2 = a2.accel;
    an.accel = [ac1,ac2](double V, distance d) {return ac1(V,d) + ac2(V,d);};*/
    return an;
}
