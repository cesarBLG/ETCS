/*
 * European Train Control System
 * Copyright (C) 2022  César Benito <cesarbema2009@hotmail.com>
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