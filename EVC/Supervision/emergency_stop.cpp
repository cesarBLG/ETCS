/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "../Procedures/mode_transition.h"
#include "../Procedures/stored_information.h"
#include "../MA/movement_authority.h"
std::map<int, optional<distance>> emergency_stops;
void handle_unconditional_emergency_stop(int id)
{
    trigger_condition(20);
    emergency_stops[id] = {};
}
int handle_conditional_emergency_stop(int id, distance location)
{
    if (d_minsafefront(location) > location)
        return 3;
    emergency_stops[id] = location;
    bool used = !SvL || *SvL > location;
    delete_MA(location, location);
    calculate_SvL();
    svl_shorten('a');
    return used ? 0 : 1;
}
void revoke_emergency_stop(int id)
{
    emergency_stops.erase(id);
    calculate_SvL();
}