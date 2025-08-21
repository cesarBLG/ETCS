/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "../Packets/radio.h"
#include "../Euroradio/session.h"
#include "../Supervision/fixed_values.h"
optional<std::pair<distance, distance>> taf_request;
bool start_display_taf;
bool stop_display_taf;
void request_track_ahead_free(distance start, double length)
{
    taf_request = {start, start+length};
    start_display_taf = false;
    stop_display_taf = false;
}
void update_track_ahead_free_request()
{
    if (taf_request && taf_request->second.est + D_keep_information < d_estfront)
        taf_request = {};
    if (taf_request) {
        if (taf_request->first.est <= d_estfront)
            start_display_taf = true;
        if (taf_request->second.est < d_estfront)
            stop_display_taf = true;
    } else {
        start_display_taf = false;
        stop_display_taf = false;
    }
}
void track_ahead_free_granted()
{
    taf_request = {};
    if (supervising_rbc) {
        supervising_rbc->queue(std::make_shared<taf_granted>());
    }
}