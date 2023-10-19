/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "../Position/distance.h"
dist_base distance_curve(const acceleration &a, const dist_base &dref, double vref, double vel);
double speed_curve(const acceleration &a, const dist_base &dref, double vref, dist_base dist);
