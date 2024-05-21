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
extern bool automatic_close_power_switch;
extern bool automatic_open_power_switch;
extern bool automatic_raise_pantograph;
extern bool automatic_lower_pantograph;
extern bool automatic_traction_system_change;
extern bool traction_cutoff_implemented;
void update_power_status();