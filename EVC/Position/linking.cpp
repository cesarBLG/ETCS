/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "linking.h"
#include "distance.h"
#include "../Supervision/national_values.h"
#include "../Packets/messages.h"
#include "../TrainSubsystems/cold_movement.h"
std::list<link_data> linking;
std::list<link_data>::iterator link_expected = linking.end();
std::list<lrbg_info> lrbgs;
bool position_valid=false;
void load_train_position()
{
    //TODO: load lrbgs
    position_valid = cold_movement_status == NoColdMovement;
}
void save_train_position()
{

}
distance update_location_reference(bg_id nid_bg, int dir, distance group_pos, bool linked, optional<link_data> link)
{
    if (!linked) {
        double offset = group_pos.get();
        distance::update_unlinked_reference(offset);
        return distance(0, group_pos.get_orientation(), offset);
    } else if (link) {
        double offset = link->dist.get();
        reset_odometer(group_pos.get());
        distance::update_distances(offset, group_pos.get());
        group_pos = distance(0, group_pos.get_orientation(), 0);
        lrbgs.push_back({nid_bg, link->reverse_dir, group_pos, link->locacc});
        if (lrbgs.size() > 10) lrbgs.pop_front();
        if (!pos_report_params || pos_report_params->LRBG)
            position_report_reasons[9] = true;
        return group_pos;
    } else {
        double offset = group_pos.get();
        reset_odometer(offset);
        distance::update_distances(offset, offset);
        lrbgs.push_back({nid_bg, dir, group_pos, Q_NVLOCACC});
        if (lrbgs.size() > 10) lrbgs.pop_front();
        if (!pos_report_params || pos_report_params->LRBG)
            position_report_reasons[9] = true;
        return distance(0, group_pos.get_orientation(), 0);
    }
}
void update_linking(distance start, Linking link, bool infill, bg_id this_bg)
{
    std::vector<LinkingElement> elements;
    elements.push_back(link.element);
    elements.insert(elements.end(), link.elements.begin(), link.elements.end());
    std::list<link_data> links;
    distance cumdist=start;
    int current_NID_C = this_bg.NID_C;
    for (LinkingElement l : elements) {
        link_data d;
        d.dist = cumdist+l.D_LINK.get_value(link.Q_SCALE);
        d.locacc = l.Q_LOCACC;
        d.nid_bg = {l.Q_NEWCOUNTRY == Q_NEWCOUNTRY_t::SameCountry ? current_NID_C : l.NID_C, (int)l.NID_BG};
        d.reaction = l.Q_LINKREACTION;
        d.reverse_dir = l.Q_LINKORIENTATION == Q_LINKORIENTATION_t::Reverse;
        current_NID_C = d.nid_bg.NID_C;
        links.push_back(d);
        cumdist = d.dist;
        for (auto &lrbg : lrbgs) {
            if (lrbg.nid_lrbg == d.nid_bg)
                lrbg.locacc = d.locacc;
        }
    }
    bool expecting_linking = link_expected != linking.end();
    bg_id expected_bg;
    if (expecting_linking)
        expected_bg = link_expected->nid_bg;
    if (infill) {
        linking.erase(linking.begin(), link_expected);
        for (auto it = linking.begin(); it!=linking.end(); ++it) {
            if (it->dist > start) {
                linking.erase(it, linking.end());
                break;
            }
        }
        linking.insert(linking.end(), links.begin(), links.end());
    } else {
        linking.erase(linking.begin(), link_expected);
        distance d_lrbg = distance(0, odometer_orientation, 0);
        for (auto it = linking.begin(); it!=linking.end(); ++it) {
            if (it->dist > d_lrbg) {
                linking.erase(it, linking.end());
                break;
            }
        }
        for (auto it = links.begin(); it!=links.end(); ++it) {
            if (it->dist > d_lrbg) {
                linking.insert(linking.end(), it, links.end());
                break;
            }
        }
    }
    bool prev_expected_found = false;
    if (expecting_linking) {
        for (auto it = linking.begin(); it!=linking.end(); ++it) {
            if (it->nid_bg == expected_bg) {
                link_expected = it;
                prev_expected_found = true;
                break;
            }
        }
    }
    if (!prev_expected_found)
        link_expected = linking.begin();
}
void delete_linking()
{
    linking.clear();
    link_expected = linking.end();
}
void delete_linking(distance d)
{
    if (link_expected != linking.end() && link_expected->dist > d)
        link_expected = linking.end();
    for (auto it = linking.begin(); it != linking.end(); ++it) {
        if (it->dist > d) {
            linking.erase(it, linking.end());
            break;
        }
    }
}