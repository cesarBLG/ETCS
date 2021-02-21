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
        g.forwards = elems[i].Q_MPOSITION == Q_MPOSITION_t::Same;
        g.initial_val = elems[i].M_POSITION.rawdata;
        if (elems[i].M_POSITION == M_POSITION_t::NoMoreCalculation)
            g.initial_val = -1;
        g.start_offset = elems[i].D_POSOFF.get_value(p.Q_SCALE);
        g.id = {elems[i].Q_NEWCOUNTRY == Q_NEWCOUNTRY_t::SameCountry ? this_bg.NID_C : elems[i].NID_C, (int)elems[i].NID_BG};
        geo_references.push_back(g);
    }
}
void geographical_position_handle_bg_passed(bg_id id, distance ref, bool reverse)
{
    for (geographical_position &p : geo_references) {
        if (!p.bg_ref && p.id == id) {
            p.bg_ref = ref;
            if (reverse) {
                p.forwards = !p.forwards;
            }
        }
    }
}
void update_geographical_position()
{
    for (auto it = geo_references.begin(); it != geo_references.end(); ++it) {
        if (it->bg_ref && *it->bg_ref+it->start_offset < d_estfront) {
            if (it->initial_val < 0)
                valid_geo_reference = {};
            else
                valid_geo_reference = *it;
            std::remove_if(geo_references.begin(), geo_references.end(), 
                [](geographical_position p){return p.bg_ref && *p.bg_ref+p.start_offset<d_estfront;});
            break;
        }
    }
    if (valid_geo_reference && valid_geo_reference->get_position(d_estfront) < 0)
        valid_geo_reference = {};
}