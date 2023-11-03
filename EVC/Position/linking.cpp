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
#include "../Procedures/level_transition.h"
#include "../TrackConditions/track_condition.h"
std::list<link_data> linking;
std::list<link_data>::iterator link_expected = linking.end();
std::list<std::pair<lrbg_info, int>> orbgs;
std::optional<lrbg_info> solr;
std::map<bg_id, double> stored_locacc;
bool position_valid=false;
void from_json(const json &pos, lrbg_info &lrbg)
{
    lrbg = {bg_id({pos["NID_C"], pos["NID_BG"]}), pos["Direction"], dist_base(pos["Position"], pos["Orientation"]), pos["Q_LOCACC"]};
}
void to_json(json &pos, const lrbg_info &lrbg)
{
    pos["NID_C"] = lrbg.nid_lrbg.NID_C;
    pos["NID_BG"] = lrbg.nid_lrbg.NID_BG;
    pos["Q_LOCACC"] = lrbg.locacc;
    pos["Direction"] = lrbg.dir;
    pos["Orientation"] = lrbg.position.orientation;
    pos["Position"] = lrbg.position.dist;
}
void load_train_position()
{
    json pos = load_cold_data("Position");
    if (!pos.is_null() && cold_movement_status == NoColdMovement) {
        if (pos.contains("LRBG")) {
            solr = pos["LRBG"];
            odometer_reference = pos["OdometerOffset"];
            orbgs.push_front({*solr, 0});
            position_valid = true;
        }
    } else {
        orbgs.clear();
        solr = {};
        position_valid = false;
        pos = nullptr;
        save_cold_data("Position", pos);
    }
}
void save_train_position()
{
    if (!orbgs.empty())
        position_valid = true;
    json pos;
    for (auto it = orbgs.begin(); it != orbgs.end(); ++it) {
        if ((it->second & 1) == 0) {
            pos["LRBG"] = it->first;
            pos["OdometerOffset"] = odometer_reference;
            break;
        }
    }
    save_cold_data("Position", pos);
}
optional<std::pair<double, double>> get_linked_bg_location(bg_id nid_bg)
{
    if (!solr)
        return {};
    if (nid_bg == solr->nid_lrbg)
        return std::pair<double,double>(0, solr->locacc);
    for (auto it = linking.begin(); it != linking.end(); ++it) {
        if (it->nid_bg == solr->nid_lrbg) {
            for (auto it2 = it; it2 != linking.end(); ++it2) {
                if (it2->nid_bg.NID_BG == NID_BG_t::Unknown)
                    break;
                if (it2->nid_bg == nid_bg)
                    return std::pair<double,double>(it2->dist-it->dist, it2->locacc);
            }
            break;
        }
        if (it->nid_bg == nid_bg) {
            for (auto it2 = it; it2 != linking.end(); ++it2) {
                if (it2->nid_bg.NID_BG == NID_BG_t::Unknown)
                    break;
                if (it2->nid_bg == solr->nid_lrbg)
                    return std::pair<double,double>(it->dist-it2->dist, it2->locacc);
            }
            break;
        }
    }
    return {};
}
void position_update_bg_passed(bg_id id, bool linked, dist_base pos, int dir)
{
#if BASELINE < 4
    if (!linked) {
        for (distance *d = distance::begin; d != nullptr; d = d->next) {
            if (d->balise_based && d->ref.dist != 0)
                d->ref = pos;
        }
    }
#endif
    if (stored_locacc.find(id) == stored_locacc.end())
        stored_locacc[id] = Q_NVLOCACC;
    orbgs.push_front({{id, dir, pos, stored_locacc[id]}, linked ? 0 : 1});
    if (linked && (!pos_report_params || pos_report_params->LRBG))
        position_report_reasons[9] = true;
    int lrbg_count = 0;
    int unlinked_count = 0;
    int count = 0;
    for (auto it = orbgs.begin(); it != orbgs.end();) {
        ++count;
        bool unlinked = (it->second & 1) != 0;
        bool buffer = (it->second & 2) != 0;
        if (!unlinked) {
            if (lrbg_count >= 8 && !buffer) {
                it = orbgs.erase(it);
                continue;
            }
            ++lrbg_count;
            if (unlinked_count > 0)
                ++unlinked_count;
        } else {
            if (unlinked_count >= 2 && !buffer) {
                it = orbgs.erase(it);
                continue;
            }
            ++unlinked_count;
        }
        if (buffer) {
            bool in = false;;
            for (auto &msg : transition_buffer) {
                if (!msg.empty() && msg.front()->nid_bg == it->first.nid_lrbg) {
                    in = true;
                    break;
                }
            }
            if (!in && ((!unlinked && lrbg_count > 8) || (unlinked && unlinked_count > 2))) {
                it = orbgs.erase(it);
                continue;
            }
        }
        ++it;
    }
}
void relocate_linking();
void relocate()
{
    if (orbgs.empty())
        return;
    bg_id newsolr;
#if BASELINE == 4
    if (link_expected == linking.end()) {
        newsolr = orbgs.front().first.nid_lrbg;
    } else {
        for (auto it = orbgs.begin(); it != orbgs.end(); ++it) {
            if (it->second & 1)
                return;
            bool link = false;
            for (auto &l : linking) {
                if (l.nid_bg == it->first.nid_lrbg) {
                    link = true;
                    break;
                }
            }
            if (link) {
                newsolr = it->first.nid_lrbg;
            }
        }
    }
#else
    bool any = false;
    for (auto it = orbgs.begin(); it != orbgs.end(); ++it) {
        if ((it->second & 1) == 0) {
            any = true;
            newsolr = it->first.nid_lrbg;
            break;
        }
    }
    if (!any)
        return;
#endif
    if (solr && solr->nid_lrbg == newsolr)
        return;
    std::optional<lrbg_info> prevsolr = solr;
    double offset = 0;
    dist_base zero;
    dist_base prev_dist;
    for (auto &rbg : orbgs) {
        if (rbg.first.nid_lrbg == newsolr) {
            zero = dist_base(0, rbg.first.position.orientation);
            prev_dist = rbg.first.position;
            offset = prev_dist - zero;
            solr = rbg.first;
            solr->position = zero;
            break;
        }
    }
    if (!solr || solr->nid_lrbg != newsolr)
        return;
    for (auto &rbg : orbgs) {
        rbg.first.position -= offset;
    }
    reset_odometer(prev_dist.dist);
    save_train_position();
    optional<std::pair<double,double>> link;
    if (prevsolr)
        link = get_linked_bg_location(prevsolr->nid_lrbg);
    if (link)
        link->first = -link->first;
#if DEBUG_ODOMETER
    std::string dbg = "Relocating at "+std::to_string(odometer_value)+"\r\n";
    if (link)
        dbg += "Linking: "+std::to_string(link->first)+"\r\n";
    dbg += "Odo: "+std::to_string(offset)+"\r\n";
#endif
    int reloc_odo=0;
#if BASELINE == 4
    int reloc_a=0;
    int reloc_b=0;
    int reloc_c=0;
#else
    int reloc_link=0;
#endif
    for (distance *d = distance::begin; d != nullptr; d = d->next) {
        if (!d->balise_based) {
            d->min -= offset;
            d->est -= offset;
            d->max -= offset;
            d->ref -= offset;
            ++reloc_odo;
            continue;
        }
#if BASELINE == 4
        if (link && !d->relocated_c) {
            d->min -= link->first;
            d->est -= link->first;
            d->max -= link->first;
            d->relocated_c = false;
            d->relocated_c_earlier = {};
            ++reloc_a;
        } else if (prevsolr) {
            bool rear = false;
            if (!d->relocated_c_earlier && link) {
                d->min -= link->first - 2 * link->second;
                d->est -= link->first;
                d->max -= link->first + 2 * link->second;
                d->relocated_c = false;
                d->relocated_c_earlier = {};
                ++reloc_b;
            } else {
                d->max -= d_maxsafefront(prevsolr->position, prevsolr->locacc) - d_maxsafefront(solr->position, solr->locacc);
                d->est -= offset;
                d->min -= d_minsafefront(prevsolr->position, prevsolr->locacc) - d_minsafefront(solr->position, solr->locacc);
                d->relocated_c = true;
                if (d->relocated_c_earlier) {
                    for (auto it = orbgs.rbegin(); it != orbgs.rend(); ++it) {
                        if (it == *orbgs)
                            break;
                        if (it == *d->relocated_c_earlier) {
                            d->relocated_c_earlier = {};
                            break;
                        }
                    }
                }
                ++reloc_c;
            }
        } else {
            abort();
        }
#else
        if (link && d->ref.dist == 0) {
            d->min -= link->first;
            d->est -= link->first;
            d->max -= link->first;
            ++reloc_link;
        } else {
            d->min -= offset;
            d->est -= offset;
            d->max -= offset;
            if (d->ref.dist != 0)
                d->ref -= offset;
            ++reloc_odo;
        }
#endif
    }
#ifdef DEBUG_ODOMETER
#if BASELINE == 4
    dbg += "Relocated a): "+reloc_a+"\r\n";
    dbg += "Relocated b): "+reloc_b+"\r\n";
    dbg += "Relocated c): "+reloc_c+"\r\n";
#else
    dbg += "Relocated agains link: "+std::to_string(reloc_link)+"\r\n";
#endif
    dbg += "Relocated against odo: "+std::to_string(reloc_odo);
    platform->debug_print(dbg);
#endif
    relocate_linking();
    extern std::map<track_condition*, std::vector<std::shared_ptr<target>>> track_condition_targets; 
    track_condition_targets.clear();
    recalculate_MRSP();
}
optional<distance> get_reference_location(bg_id bg, bool linked, bool check_passed)
{
    if (solr) {
        if (solr->nid_lrbg == bg)
            return distance(solr->position.dist, solr->position.orientation);
        if (linked) {
            bool passed = false;
            if (check_passed) {
                for (auto it = orbgs.begin(); it != orbgs.end(); ++it) {
                    if (it->first.nid_lrbg == bg) {
                        passed = true;
                        break;
                    }
                }
            }
            if (passed || !check_passed) {
                auto link = get_linked_bg_location(bg);
                if (link)
                    return distance(link->first, solr->position.orientation);
            }
        }
    }
#if BASELINE == 4
    bool solr_ahead = false;
    for (auto it = orbgs.begin(); it != orbgs.end(); ++it) {
        if (solr && it->first.nid_lrbg == solr->nid_lrbg)
            solr_ahead = true;
        if (solr && it->first.nid_lrbg == bg) {
            distance d;
            d.max -= d_maxsafefront(it->first.position, it->first.locacc) - d_maxsafefront(solr->position, solr->locacc);
            d.est -= dist_base(0, 0) - it->first.position;
            d.min -= d_minsafefront(it->first.position, it->first.locacc) - d_minsafefront(solr->position, solr->locacc);
            d.ref = dist_base(0, 0);
            d.relocated_c = true;
            if (!solr_ahead)
                d.relocated_c_earlier = bg;
            return d;
        }
    }
#else
    for (auto it = orbgs.begin(); it != orbgs.end(); ++it) {
        if (it->first.nid_lrbg == bg) {
            distance d;
            d.min = d.max = d.est = it->first.position;
            if (!linked)
                d.ref = d.est;
            else
                d.ref = dist_base(0, 0);
            return d;
        }
    }
#endif
    return {};
}
void relocate_linking()
{
    if (!solr || linking.empty())
        return;
    bool found=false;
    for (auto it = linking.begin(); it != linking.end(); ++it) {
        if (it->nid_bg == solr->nid_lrbg) {
            found = true;
            linking.erase(linking.begin(), it);
            break;
        }
        if (it == link_expected)
            ++link_expected;
    }
    if (!found) {
        linking.clear();
        link_expected = linking.end();
    }
    if (linking.empty())
        return;
    dist_base ref = linking.front().dist;
    dist_base newref = dist_base(0, ref.orientation);
    double offset = ref-newref;
    for (auto it = linking.begin(); it != linking.end(); ++it) {
        it->dist -= offset;
    }
}
void update_linking(Linking link, bool infill, bg_id ref_bg)
{
    std::vector<LinkingElement> elements;
    elements.push_back(link.element);
    elements.insert(elements.end(), link.elements.begin(), link.elements.end());

    bool found = false;
    for (auto it = linking.begin(); it != linking.end(); ++it) {
        if (it->nid_bg == ref_bg) {
            found = true;
            linking.erase(++it, linking.end());
            break;
        }
    }
    if (!found) {
        linking.clear();
        link_data d;
        if (stored_locacc.find(ref_bg) == stored_locacc.end())
            stored_locacc[ref_bg] = Q_NVLOCACC;
        d.locacc = stored_locacc[ref_bg];
        d.nid_bg = ref_bg;
        d.reaction = 2;
        for (auto it = orbgs.begin(); it != orbgs.end(); ++it) {
            if (it->first.nid_lrbg == ref_bg) {
                d.dist = it->first.position;
                found = true;
            }
        }
        if (!found)
            abort();
        linking.push_back(d);
    }
    dist_base cumdist = linking.back().dist;
    std::list<link_data> links;
    int current_NID_C = ref_bg.NID_C;
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
        if (stored_locacc.find(d.nid_bg) == stored_locacc.end())
            stored_locacc[d.nid_bg] = d.locacc;
    }
    linking.insert(linking.end(), links.begin(), links.end());
    start_checking_linking();
}
void start_checking_linking()
{
    link_expected = linking.end();
#if BASELINE == 4
    if (mode != Mode::FS && mode != Mode::OS && mode != Mode::LS && mode != Mode::AD && mode != Mode::SM)
#else
    if (mode != Mode::FS && mode != Mode::OS && mode != Mode::LS)
        return;
#endif
    for (auto it = linking.begin(); it!=linking.end(); ++it) {
        bool passed=false;
        for (auto it2 = orbgs.begin(); it2 != orbgs.end(); ++it2) {
            if (it2->first.nid_lrbg == it->nid_bg) {
                passed = true;
                break;
            }
        }
        if (passed)
            continue;
        if (it->max() > d_minsafefront(confidence_data::basic()) - L_antenna_front) {
            link_expected = it;
            break;
        }
    }
}
void delete_linking()
{
    linking.clear();
    link_expected = linking.end();
}
void delete_linking(const distance &d)
{
    if (link_expected != linking.end() && link_expected->min() > d.min)
        link_expected = linking.end();
    for (auto it = linking.begin(); it != linking.end(); ++it) {
        if (it->dist > d.min) {
            linking.erase(it, linking.end());
            break;
        }
    }
}