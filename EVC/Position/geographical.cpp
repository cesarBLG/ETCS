/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "geographical.h"
#include <algorithm>
optional<geographical_position> valid_geo_reference;
std::list<geographical_position> geo_references;
void handle_geographical_position(GeographicalPosition p, bg_id this_bg)
{
    geo_references.clear();
    std::vector<GeographicalPosition_element_packet> elems;
    elems.push_back(p.element);
    elems.insert(elems.end(), p.elements.begin(), p.elements.end());
    for (int i=0; i<elems.size(); i++) {
        geographical_position g;
        g.forwards = elems[i].Q_MPOSITION == elems[i].Q_MPOSITION.Same;
        g.initial_val = elems[i].M_POSITION.rawdata;
        if (elems[i].M_POSITION == elems[i].M_POSITION.NoMoreCalculation)
            g.initial_val = -1;
        g.start_offset = elems[i].D_POSOFF.get_value(p.Q_SCALE);
        g.id = {elems[i].Q_NEWCOUNTRY == elems[i].Q_NEWCOUNTRY.SameCountry ? this_bg.NID_C : elems[i].NID_C, (int)elems[i].NID_BG};
        geo_references.push_back(g);
    }
}
void geographical_position_handle_bg_passed(bg_id id, dist_base ref, bool reverse)
{
    for (geographical_position &p : geo_references) {
        if (!p.bg_ref && p.id == id) {
            p.bg_ref = distance::from_odometer(ref);
            if (reverse) {
                p.forwards = !p.forwards;
            }
        }
    }
}
void update_geographical_position()
{
    for (auto it = geo_references.begin(); it != geo_references.end(); ++it) {
        if (it->bg_ref && it->bg_ref->est+it->start_offset < d_estfront) {
            if (it->initial_val < 0)
                valid_geo_reference = {};
            else
                valid_geo_reference = *it;
            geo_references.remove_if(
                [](geographical_position p){return p.bg_ref && p.bg_ref->est+p.start_offset<d_estfront;});
            break;
        }
    }
    if (valid_geo_reference && valid_geo_reference->get_position(d_estfront) < 0)
        valid_geo_reference = {};
}