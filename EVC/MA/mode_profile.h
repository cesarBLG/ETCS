/*
 * European Train Control System
 * Copyright (C) 2020  César Benito <cesarbema2009@hotmail.com>
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
#include <list>
#include "../Packets/80.h"
#include "../Position/distance.h"
#include "../Supervision/national_values.h"
#include "../TrainSubsystems/brake.h"
#include "../optional.h"
struct mode_profile
{
    distance start;
    double length;
    double acklength;
    Mode mode;
    bool start_SvL;
    double speed;
};
extern std::list<mode_profile> mode_profiles;
extern bool in_mode_ack_area;
extern bool mode_timer_started;
extern int64_t mode_timer;
extern optional<mode_profile> requested_mode_profile;
extern optional<int64_t> display_lssma_time;
extern double lssma;
extern bool display_lssma;
extern bool ls_function_marker;
void update_mode_profile();
void reset_mode_profile(distance ref, bool infill);
void set_mode_profile(ModeProfile profile, distance ref, bool infill);