/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "train_trip.h"
#include "../MA/movement_authority.h"
#include "mode_transition.h"
#include "../STM/stm.h"
#include "../language/language.h"
#include <string>
bool trip_acknowledged = false;
bool trip_exit_acknowledged = false;
void train_trip(int reason)
{
    std::string str;
    switch(reason) {
        case 12:
        case 16:
        case 18:
        case 43:
            str = get_text("Unauthorized passing of EoA/LoA");
            break;
        case 17:
        case 66:
            str = get_text("Balise read error");
            break;
        case 39:
        case 67:
            str = get_text("No MA received at level transition");
            break;
        case 41:
            str = get_text("Communication error");
            break;
        case 35:
            str = get_text("SH refused");
            break;
        case 65:
            str = get_text("Trackside not compatible");
            break;
        case 42:
            str = get_text("SR distance exceeded");
            break;
        case 49:
        case 52:
            str = get_text("SH stop order");
            break;
        case 36:
        case 54:
            str = get_text("SR stop order");
            break;
        case 20:
            str = get_text("Emergency stop");
            break;
        case 69:
            str = get_text("No track description");
            break;
        case 38: {
            std::string name = "NTC";
            for (auto &kvp : installed_stms) {
                if (kvp.second->national_trip) {
                    auto it = ntc_names.find(kvp.first);
                    if (it != ntc_names.end())
                        name = it->second;
                }
            }
            str = name+get_text(" brake demand");
            break;
        }
        default:
            break;
    }
    add_message(text_message(str, true, false, 1, [](text_message &t){return mode != Mode::TR && mode != Mode::PT;}));
}
void update_trip()
{
    if (mode != Mode::TR) {
        trip_acknowledged = false;
        return;
    }
    if (V_est<=0 && !trip_acknowledged) {
        mode_to_ack = Mode::TR;
        mode_acknowledgeable = true;
        mode_acknowledged = false;
        trip_acknowledged = true;
        trip_exit_acknowledged = false;
    }
}