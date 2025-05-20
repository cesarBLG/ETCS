/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "route_suitability.h"
#include "../optional.h"
#include "../MA/movement_authority.h"
#include "../Supervision/train_data.h"
#include "../DMI/text_message.h"
#include "../language/language.h"
std::map<int, distance> route_suitability;
void load_route_suitability(RouteSuitabilityData &data, distance ref)
{
    if (data.Q_TRACKINIT == data.Q_TRACKINIT.InitialState) {
        distance resume = ref + data.D_TRACKINIT.get_value(data.Q_SCALE);
        for (auto it = route_suitability.begin(); it != route_suitability.end(); ) {
            if (it->second.max > resume.min)
                it = route_suitability.erase(it);
            else
                ++it;
        }
        return;
    }
    std::vector<RouteSuitability_element> elements;
    elements.push_back(data.element);
    elements.insert(elements.end(), data.elements.begin(), data.elements.end());
    for (auto &el : elements) {
        ref += el.D_SUITABILITY.get_value(data.Q_SCALE);
        if (el.Q_SUITABILITY.rawdata == el.Q_SUITABILITY.LoadingGauge) {
            bool supported = false;
            supported |= ((el.M_LINEGAUGE.rawdata >> el.M_LINEGAUGE.BitG1) & 1) && loading_gauge == loading_gauges::G1;
            supported |= ((el.M_LINEGAUGE.rawdata >> el.M_LINEGAUGE.BitGA) & 1) && loading_gauge == loading_gauges::GA;
            supported |= ((el.M_LINEGAUGE.rawdata >> el.M_LINEGAUGE.BitGB) & 1) && loading_gauge == loading_gauges::GB;
            supported |= ((el.M_LINEGAUGE.rawdata >> el.M_LINEGAUGE.BitGC) & 1) && loading_gauge == loading_gauges::GC;
            if (supported)
                route_suitability.erase(0);
            else
                route_suitability[0] = ref;
        } else if (el.Q_SUITABILITY.rawdata == el.Q_SUITABILITY.MaxAxleLoad) {
            bool supported = el.M_AXLELOADCAT.rawdata >= (int)axle_load_category;
            if (supported)
                route_suitability.erase(1);
            else
                route_suitability[1] = ref;
        } else if (el.Q_SUITABILITY.rawdata == el.Q_SUITABILITY.TractionSystem) {
            bool supported = false;
            for (auto &t : traction_systems) {
                if (t.electrification == el.M_VOLTAGE.rawdata && (el.M_VOLTAGE.rawdata == 0 || t.nid_ctraction == el.NID_CTRACTION)) {
                    supported = true;
                    break;
                }
            }
            if (supported)
                route_suitability.erase(2);
            else
                route_suitability[2] = ref;
        }
    }
    for (auto &rs : route_suitability) {
        int type = rs.first;
        std::string text;
        switch (type) {
            case 0:
                text = get_text("Route suitability - loading gauge");
                break;
            case 1:
                text = get_text("Route suitability - axle load category");
                break;
            case 2:
                text = get_text("Route suitability - traction system");
                break;
        }
        add_message(text_message(text, true, false, 0, [type](text_message &m) {
            return route_suitability.find(type) == route_suitability.end();
        }));
    }
}
void update_route_suitability()
{
}