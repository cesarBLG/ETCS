/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
extern optional<Mode> in_mode_ack_area;
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