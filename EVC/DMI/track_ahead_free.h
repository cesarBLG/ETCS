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
#include "../optional.h"
extern optional<std::pair<distance, distance>> taf_request;
extern bool start_display_taf;
extern bool stop_display_taf;
void request_track_ahead_free(distance start, double length);
void track_ahead_free_granted();
void update_track_ahead_free_request();