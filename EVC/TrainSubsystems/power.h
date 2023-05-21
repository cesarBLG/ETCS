/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "../Position/distance.h"
#include "../optional.h"
extern bool main_power_on_available;
extern bool main_power_off_available;
extern bool raise_pantograph_available;
extern bool lower_pantograph_available;
extern bool traction_cutoff_available;
void update_power_status();