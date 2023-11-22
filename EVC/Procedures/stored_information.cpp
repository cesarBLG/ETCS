/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "../Supervision/speed_profile.h"
#include "../Position/linking.h"
#include "stored_information.h"
#include "../MA/movement_authority.h"
#include "../Supervision/national_values.h"
#include "../Packets/radio.h"
#include "../Procedures/mode_transition.h"
#include "../TrackConditions/track_condition.h"
#include "../TrackConditions/route_suitability.h"
void shorten(bool include_ma, distance d)
{
    if (not_yet_applicable_nv && not_yet_applicable_nv->first_applicable.max > d.min)
        not_yet_applicable_nv = {};
    delete_linking(d);
    if (include_ma)
        delete_MA(distance::from_odometer(d_estfront_dir[odometer_orientation == -1]), d);
    delete_gradient(d);
    delete_SSP(d);
    if (mode != Mode::SH)
        sh_balises = {};
    for (auto it = mode_profiles.begin(); it != mode_profiles.end();) {
        if (it->start.max > d.min)
            it = mode_profiles.erase(it);
        else
            ++it;
    }
    if (accepting_rbc && rbc_transition_position.max > d.min)
        accepting_rbc = handing_over_rbc = nullptr;
    for (auto it = route_suitability.begin(); it != route_suitability.end();) {
        if (it->second.max > d.min)
            it = route_suitability.erase(it);
        else
            ++it;
    }
    for (auto it = track_conditions.begin(); it != track_conditions.end();) {
        if (it->get()->start.max > d.min)
            it = track_conditions.erase(it);
        else
            ++it;
    }
    delete_PBD(d);
}
void svl_shorten(char condition)
{
    if (!MA || !MA->SvL_ma) return;
    platform->debug_print("MA shortening "+std::string(1,condition));
    shorten(false, *MA->SvL_ma);
    if (condition != 'a' && condition != 'b' && condition != 'f' && (level == Level::N2 || level == Level::N3))
        ma_rq_reasons[3] = true;
}
void train_shorten(char condition)
{
    platform->debug_print("Shortening to front "+std::string(1,condition));
    shorten(true, distance::from_odometer(d_maxsafefront(confidence_data::basic())));
    if (level == Level::N2 || level == Level::N3)
        ma_rq_reasons[3] = true;
}
void desk_closed_som()
{
    not_yet_applicable_nv = {};
    delete_information(Mode::SB);
}