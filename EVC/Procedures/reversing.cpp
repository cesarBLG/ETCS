/*
 * European Train Control System
 * Copyright (C) 2024  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "reversing.h"
#include "mode_transition.h"
#include "../TrainSubsystems/train_interface.h"
std::optional<reversing_supervision> rv_supervision;
std::optional<reversing_area> rv_area;
std::optional<distance> rv_area_end_original;
bool reversing_permitted;
void update_reversing()
{
    reversing_permitted = (mode == Mode::OS || mode == Mode::LS || mode == Mode::FS) && V_est == 0 && rv_area && rv_supervision && d_estfront > rv_area->start.est && d_estfront < rv_area->end.est;
    if (reversing_permitted && reverser_direction == -1) {
        if (mode_to_ack != Mode::RV || !mode_acknowledgeable) {
            mode_to_ack = Mode::RV;
            mode_acknowledgeable = true;
            mode_acknowledged = false;
        }
    } else if (mode != Mode::RV && mode_to_ack == Mode::RV) {
        mode_acknowledgeable = false;
        mode_acknowledged = false;
        mode_to_ack = mode;
    }
    if (mode == Mode::RV && !rv_position)
        rv_position = *rv_area_end_original-rv_supervision->max_distance;
}