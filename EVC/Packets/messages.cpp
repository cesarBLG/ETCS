/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "messages.h"
#include "information.h"
#include "radio.h"
#include "logging.h"
#include "vbc.h"
#include "0.h"
#include "12.h"
#include "21.h"
#include "27.h"
#include "41.h"
#include "42.h"
#include "58.h"
#include "65.h"
#include "131.h"
#include "136.h"
#include "180.h"
#include "../Supervision/emergency_stop.h"
#include "../Procedures/mode_transition.h"
#include "../Procedures/level_transition.h"
#include "../Procedures/override.h"
#include "../Procedures/start.h"
#include "../Procedures/train_trip.h"
#include "../Position/geographical.h"
#include "../TrainSubsystems/brake.h"
#include "../TrainSubsystems/train_interface.h"
#include "../DMI/windows.h"
#include "../Version/version.h"
#include "../Version/translate.h"
#include <algorithm>
#include "platform_runtime.h"
static int reading_nid_bg=-1;
static int reading_nid_c=-1;
static std::vector<eurobalise_telegram> telegrams;
static distance last_passed_distance;
static bool reffound=false;
static bool refmissed=false;
static bool dupfound=false;
static bool refpassed=false;
static bool linked = false;
static int prevpig=-1;
static int totalbg=8;
static bool reading = false;
static int rams_lost_count = 0;
static int dir = -1;
static int orientation = -1;
static int64_t first_balise_time;
static distance bg_reference1;
static distance bg_reference1max;
static distance bg_reference1min;
static distance bg_reference;
static distance bg_referencemax;
static distance bg_referencemin;
static std::optional<link_data> reading_bg_link;
static bool linking_available;
std::deque<std::pair<eurobalise_telegram, std::pair<distance,int64_t>>> pending_telegrams;
optional<link_data> rams_reposition_mitigation;
void trigger_reaction(int reaction);
void handle_telegrams(std::vector<eurobalise_telegram> message, dist_base dist, int dir, int64_t timestamp, bg_id nid_bg, bool linked, int m_version);
void handle_radio_message(std::shared_ptr<euroradio_message> message);
void check_valid_data(std::vector<eurobalise_telegram> telegrams, dist_base bg_reference, bool linked, int64_t timestamp);
void update_track_comm();
void balise_group_passed();
void check_linking();
void expect_next_linking();
std::vector<etcs_information*> construct_information(ETCS_packet *packet, euroradio_message *msg);
void trigger_reaction(int reaction)
{
    switch (reaction) {
        case 1:
            if (mode != Mode::SL && mode != Mode::PT && mode != Mode::NL && mode != Mode::RV && mode != Mode::PS)
                trigger_brake_reason(0);
            break;
        case 2:
            //No reaction
            break;
        default:
            //Train Trip - Balise read error - Linking"
            trigger_condition(17);
            break;
    }
}
void reset_eurobalise_data()
{
    telegrams.clear();
    reading_nid_bg = -1;
    reading_nid_c=-1;
    prevpig = -1;
    totalbg = 8;
    reading = false;
    dir = -1;
    orientation = -1;
    refmissed = false;
    reffound = false;
    dupfound = false;
    refpassed = false;
    linked = false;
    reading_bg_link = {};
}
void expect_next_linking()
{
    if (link_expected != linking.end())
        ++link_expected;
}
void check_linking()
{
#if BASELINE == 4
    if (mode != Mode::FS && mode != Mode::OS && mode != Mode::LS && mode != Mode::AD && mode != Mode::SM) {
#else
    if (mode != Mode::FS && mode != Mode::OS && mode != Mode::LS) {
#endif
        link_expected = linking.end();
        rams_lost_count = 0;
        rams_reposition_mitigation = {};
        reading_bg_link = {};
        linking_available = false;
        return;
    }
    if (!linking_available) {
        start_checking_linking();
        linking_available = true;
    }
    if (rams_reposition_mitigation && rams_reposition_mitigation->max() < d_minsafefront(confidence_data::basic())-L_antenna_front)
        rams_reposition_mitigation = {};
    if (link_expected == linking.end())
        rams_lost_count = 0;
    if (link_expected!=linking.end()) {
        auto link_bg=linking.end();
        for (auto it = link_expected; it!=linking.end(); ++it) {
            if (it->nid_bg == bg_id({reading_nid_c, reading_nid_bg}) || (it->nid_bg.NID_C == reading_nid_c && it->nid_bg.NID_BG == 16383)) {
                link_bg = it;
                break;
            }
        }
        bool isexpected = linked && refpassed && link_expected==link_bg;
        bool c1 = isexpected && link_expected->min() > bg_referencemax.max && link_expected->nid_bg.NID_BG != 16383;
        bool c2 = (!isexpected || link_expected->max() < bg_referencemin.min) && link_expected->max() < d_minsafefront(confidence_data::basic())-L_antenna_front;
        bool c3 = linked && link_bg!=linking.end() && link_bg != link_expected;
        if (c1 || c2 || c3) {
#ifdef DEBUG_MSG_CONSISTENCY
            platform->debug_print("Balise read error: check_linking() c1=" + std::to_string(c1) + " c2=" + std::to_string(c2) + " c3=" + std::to_string(c3));
#endif
            if (c2 || c3)
                rams_lost_count++;
            if (rams_lost_count > 1 && link_expected->reaction == 2 && (c2 || c3)) {
                trigger_reaction(1);
                rams_lost_count = 0;
                if (supervising_rbc)
                    supervising_rbc->report_error(7);
#ifdef DEBUG_MSG_CONSISTENCY
                platform->debug_print("RAMS supervision");
#endif
            } else {
                if (supervising_rbc)
                    supervising_rbc->report_error(0);
                trigger_reaction(link_expected->reaction);
            }
            if (rams_lost_count > 1)
                rams_lost_count = 0;

            reading_bg_link = {};

            expect_next_linking();
            if (c3)
                check_linking();
        } else if (linked && link_expected==link_bg && refpassed) {
            if (refpassed && bg_referencemax.max>=link_expected->min() && bg_referencemin.min<=link_expected->max())
                rams_lost_count = 0;
            reading_bg_link = *link_bg;
            expect_next_linking();
        }
    }
}
void balise_group_passed()
{
    if (!reffound) {
        refmissed = true;
        if (dupfound) {
            bg_reference = bg_reference1;
            bg_referencemax = bg_reference1max;
            bg_referencemin = bg_reference1min;
            refpassed = true;
        }
    }
    check_linking();
    bool linking_rejected=false;
    if (linked && refpassed && reading_bg_link) {
        linking_rejected = true;
        auto &l = *reading_bg_link;
        if (l.nid_bg == bg_id({reading_nid_c, reading_nid_bg})) {
            if (dir != -1 && dir != l.reverse_dir) {
#ifdef DEBUG_MSG_CONSISTENCY
                platform->debug_print("Balise error: group passed in wrong direction. Expected " + std::to_string(l.reverse_dir) + ", passed " + std::to_string(dir));
#endif
                if (supervising_rbc)
                    supervising_rbc->report_error(0);
                trigger_condition(66);
            }
            rams_reposition_mitigation = {};
            if (l.max() >= bg_referencemin.min && l.min() <= bg_referencemax.max)
                linking_rejected = false;
        }
        if (l.nid_bg.NID_BG == 16383) {
            bool repositioning = false;
            for (auto tel : telegrams) {
                for (auto pack : tel.packets) {
                    if (pack->NID_PACKET == 16) {
                        auto &Q_DIR = ((ETCS_directional_packet*)pack.get())->Q_DIR;
                        if (Q_DIR == Q_DIR.Both || (Q_DIR == Q_DIR.Nominal && dir == 0) || (Q_DIR == Q_DIR.Reverse && dir == 1)) {
                            repositioning = true;
                            break;
                        }
                    }
                }
                if (repositioning)
                    break;
            }
            if (dir != -1 && dir == l.reverse_dir && repositioning && l.max() >= bg_referencemin.min) {
                linking_rejected = false;
#if BASELINE == 4
                if (dir == -1 || dir == l.reverse_dir) {
#ifdef DEBUG_MSG_CONSISTENCY
                    platform->debug_print("Linking error expecting repositioning information");
#endif
                    if (supervising_rbc)
                        supervising_rbc->report_error(0);
                    trigger_reaction(l.reaction);
                }
#endif
            }
        }
        if (rams_reposition_mitigation) {
            bool repositioning = false;
            for (auto tel : telegrams) {
                for (auto pack : tel.packets) {
                    if (pack->NID_PACKET == 16) {
                        auto& Q_DIR = ((ETCS_directional_packet*)pack.get())->Q_DIR;
                        if (Q_DIR == Q_DIR.Both || (Q_DIR == Q_DIR.Nominal && dir == 0) || (Q_DIR == Q_DIR.Reverse && dir == 1)) {
                            repositioning = true;
                            break;
                        }
                    }
                }
                if (repositioning)
                    break;
            }
            if (dir != -1 && dir == rams_reposition_mitigation->reverse_dir && repositioning && rams_reposition_mitigation->max() >= bg_referencemin.min) {
                linking_rejected = true;
                rams_reposition_mitigation = {};
#ifdef DEBUG_MSG_CONSISTENCY
                platform->debug_print("RAMS reposition mitigation");
#endif
                if (supervising_rbc)
                    supervising_rbc->report_error(8);
                trigger_reaction(1);
            }
        }
    }
    if (!linking_rejected) check_valid_data(telegrams, bg_reference.est, linked, first_balise_time);
    reset_eurobalise_data();
}
void update_track_comm()
{
    update_radio();
    if (pending_telegrams.empty()) {
        check_linking();
        if (reading) {
            double elapsed = d_estfront_dir[odometer_orientation == -1]-L_antenna_front-last_passed_distance.est;
            if (elapsed > 12)
                balise_group_passed();
        }
    } else {
        eurobalise_telegram t = pending_telegrams.front().first;
        distance passed_dist = pending_telegrams.front().second.first-L_antenna_front;
        log_message(t, passed_dist.est, pending_telegrams.front().second.second);
        pending_telegrams.pop_front();
        extern optional<distance> rmp_position;
        int rev = ((mode == Mode::PT || mode == Mode::RV) ? -1 : 1);
        if (rmp_position && (rmp_position->est - d_estfront)*rev > 0.1) {
            update_track_comm();
            return;
        }
        if (!t.readerror) {
            int m_version = t.M_VERSION;
            if (VERSION_X(m_version) == 0) {
                update_track_comm();
                return;
            }
            bool higherver = true;
            for (int ver : supported_versions) {
                if (VERSION_X(ver) >= VERSION_X(m_version))
                    higherver = false;
            }
            if (higherver) {
                if (mode != Mode::RV)
                    trigger_condition(65);
                update_track_comm();
                return;
            }
            bool ignored = false;
            for (auto p : t.packets) {
                if (p->NID_PACKET == 0 || p->NID_PACKET == 200) {
                    auto *vbc = (VirtualBaliseCoverMarker*)p.get();
                    if (vbc_ignored(t.NID_C, vbc->NID_VBCMK))
                        ignored = true;
                } else {
                    break;
                }
            }
            if (ignored) {
                update_track_comm();
                return;
            }
        }
        distance prev_distance = last_passed_distance;
        last_passed_distance = passed_dist;
        reading = true;
        if (!t.readerror) {
            linked = t.Q_LINK == t.Q_LINK.Linked;
            if (reading_nid_bg != -1 && reading_nid_bg != t.NID_BG) {
                balise_group_passed();
                reading = true;
            }
            if (reading_nid_bg != t.NID_BG) {
                first_balise_time = get_milliseconds();
            }
            reading_nid_bg = t.NID_BG;
            reading_nid_c = t.NID_C;
            totalbg = t.N_TOTAL+1;
            if (prevpig != -1) {
                if (dir == -1)
                    dir = (prevpig>t.N_PIG) ? 1 : 0;
                if (orientation == -1)
                    orientation = (passed_dist.est<prev_distance.est) ? 1 : 0;
                if (t.N_PIG>prevpig && !reffound)
                    refmissed = true;
            }
            prevpig = t.N_PIG;
            if (t.N_PIG == 1 && t.M_DUP == t.M_DUP.DuplicateOfPrev) {
                dupfound = true;
                bg_reference1 = passed_dist;
                bg_reference1max = distance::from_odometer(d_maxsafe(passed_dist.est, confidence_data::basic()));
                bg_reference1min = distance::from_odometer(d_minsafe(passed_dist.est, confidence_data::basic()));
            }
            if (t.N_PIG == 0) {
                reffound = true;
                bg_reference = passed_dist;
                bg_referencemax = distance::from_odometer(d_maxsafe(passed_dist.est, confidence_data::basic()));
                bg_referencemin = distance::from_odometer(d_minsafe(passed_dist.est, confidence_data::basic()));
                refpassed = true;
                check_linking();
            }
            if ((dir==0 && t.N_PIG == t.N_TOTAL) || (dir == 1 && t.N_PIG == 0) || (t.N_PIG == 0 && t.N_TOTAL == 0)) {
                telegrams.push_back(t);
                balise_group_passed();
                return;
            }
        }
        telegrams.push_back(t);
        if (refmissed && dupfound) {
            bg_reference = bg_reference1;
            bg_referencemax = bg_reference1max;
            bg_referencemin = bg_reference1min;
            refpassed = true;
            check_linking();
        }
        update_track_comm();
    }
}
void check_valid_data(std::vector<eurobalise_telegram> telegrams, dist_base bg_reference, bool linked, int64_t timestamp)
{
    int nid_bg=-1;
    int nid_c=-1;
    int m_version=-1;
    int passed_dir=-1;
    int orientated_dir=-1;
    int prevno=-1;
    int n_total=-1;
    std::vector<eurobalise_telegram> read_telegrams;
    for (int i=0; i<telegrams.size(); i++) {
        eurobalise_telegram t = telegrams[i];
        if (!t.readerror) {
            m_version = t.M_VERSION;
            nid_bg = t.NID_BG;
            nid_c = t.NID_C;
            if (prevno == -1)
                prevno = t.N_PIG;
            else
                passed_dir = t.N_PIG<prevno;
            n_total = t.N_TOTAL;
            read_telegrams.push_back(t);
        }
    }
    if (nid_bg >= 0 && !overrideProcedure) {
        if (sr_balises && sr_balises->find({nid_c, nid_bg}) == sr_balises->end())
            trigger_condition(36);
        if (sh_balises && sh_balises->find({nid_c, nid_bg}) == sh_balises->end())
            trigger_condition(52);
    }
    if (dir == 1)
        std::reverse(read_telegrams.begin(), read_telegrams.end());

    if (orientation == -1) {
        if (odometer_direction == -odometer_orientation)
            orientation = 1;
        else if (odometer_direction == odometer_orientation)
            orientation = 0;
    }
    if (orientation == 1 && passed_dir != -1)
        orientated_dir = 1-passed_dir;
    else
        orientated_dir = passed_dir;
    
    int dir = orientated_dir;
    if (orientation == 1) {
        if (mode == Mode::SH || mode == Mode::PS || mode == Mode::SL) {
            bg_reference.orientation = -odometer_orientation;
            dir = passed_dir;
        }
    }

    bool accepted1 = true;
    if (read_telegrams.size() == 0)
        accepted1 = false;

    if (linked && link_expected != linking.end() && !reading_bg_link)
        return;

    if (n_total == -1) {
        accepted1 = false;
    } else {
        for (int pig=0; pig<=n_total; pig++) {
            bool reject=true;
            for (int i=0; i<read_telegrams.size() && reject; i++) {
                eurobalise_telegram t = read_telegrams[i];
                if (t.N_PIG == pig) {
                    reject = false;
                } else if ((t.M_DUP == t.M_DUP.DuplicateOfNext && t.N_PIG+1==pig) || (t.M_DUP == t.M_DUP.DuplicateOfPrev && t.N_PIG==pig+1)) {
                    if (reading_bg_link) {
                        reject = false;
                    } else {
                        if (passed_dir!=-1) {
                            reject = false;
                        } else {
                            bool directional = false;
                            for (int j=0; j<t.packets.size(); j++) {
                                ETCS_packet *p = t.packets[i].get();
                                if (p->directional && ((ETCS_directional_packet*)p)->Q_DIR != ((ETCS_directional_packet*)p)->Q_DIR.Both)
                                    directional = true;
                            }
                            if (!directional)
                                reject = false;
                        }
                    }
                    break;
                }
            }
            if (reject)
                accepted1 = false;
        }
    }

    std::vector<eurobalise_telegram> message;
    for (int i=0; i<read_telegrams.size(); i++) {
        eurobalise_telegram t = read_telegrams[i];
        bool c1 = t.M_DUP == t.M_DUP.NoDuplicates;
        bool c2 = t.M_DUP == t.M_DUP.DuplicateOfNext && i+1<read_telegrams.size() && t.N_PIG+1==read_telegrams[i+1].N_PIG;
        bool c3 = t.M_DUP == t.M_DUP.DuplicateOfPrev && i>1 && t.N_PIG==read_telegrams[i-1].N_PIG+1;
        if (c1 || !(c2||c3))
            message.push_back(t);
        if ((c2 && passed_dir==0) || (c3 && passed_dir==1)) {
            eurobalise_telegram first = t;
            eurobalise_telegram second = c2 ? read_telegrams[i+1] : read_telegrams[i-1];
            bool firstdefault = false;
            for (int j=0; j<first.packets.size(); j++) {
                if (first.packets[j]->NID_PACKET == 254) {
                    firstdefault = true;
                    break;
                }
            }
            bool seconddefault = false;
            for (int j=0; j<second.packets.size(); j++) {
                if (second.packets[j]->NID_PACKET == 254) {
                    seconddefault = true;
                    break;
                }
            }
            message.push_back(seconddefault && !firstdefault ? first : second);
        }
    }
    std::vector<std::shared_ptr<ETCS_packet>> packets;
    for (auto &t : message) {
        packets.insert(packets.end(), t.packets.begin(), t.packets.end());
    }
    for (auto &t : message) {
        auto tpacks = t.packets;
        t.packets.clear();
        for (auto &p : tpacks) {
            auto p2 = translate_packet(p, packets, m_version);
            if (p2 != nullptr)
                t.packets.push_back(p2);
        }
    }
    bool accepted2=true;
    int mcount=-1;
    for (int i=0; i<message.size(); i++) {
        if (!message[i].valid)
            accepted2 = false;
        if (message[i].M_MCOUNT == message[i].M_MCOUNT.NeverFitsTelegrams) {
            accepted2 = false;
        } else if(message[i].M_MCOUNT != message[i].M_MCOUNT.FitsAllTelegrams) {
            if (mcount==-1)
                mcount = message[i].M_MCOUNT;
            else if (mcount != message[i].M_MCOUNT)
                accepted2 = false;
        }
    }
    bool accepted = accepted1 && accepted2;
    if (!accepted) {
        if (reading_bg_link || (nid_bg >= 0 && link_expected != linking.end() && link_expected->min() < d_maxsafefront(confidence_data::basic()) - L_antenna_front && link_expected->max() > d_minsafefront(confidence_data::basic())-L_antenna_front)) {
            if (!reading_bg_link || (link_expected != linking.end() && reading_bg_link->nid_bg == link_expected->nid_bg))
                expect_next_linking();
#ifdef DEBUG_MSG_CONSISTENCY
            platform->debug_print("Balise error. Linked BG not accepted. accepted1="+std::to_string(accepted1)+", accepted2="+std::to_string(accepted2)+(reading_bg_link ? "" : ", unknown reference"));
#endif
            if (supervising_rbc)
                supervising_rbc->report_error(1);
            trigger_reaction(reading_bg_link->reaction);
        } else {
            if (accepted2) {
                for (int i=0; i<message.size(); i++) {
                    eurobalise_telegram t = message[i];
                    if (t.packets.empty())
                        continue;
                    for (int j=0; j<t.packets.size()-1; j++) {
                        if (t.packets[j]->NID_PACKET == 145) {
                            auto &Q_DIR = ((ETCS_directional_packet*)t.packets[j].get())->Q_DIR;
                            if (Q_DIR == Q_DIR.Both || (Q_DIR == Q_DIR.Nominal && dir == 0) || (Q_DIR == Q_DIR.Reverse && dir == 1))
                                return;
                        }
                    }
                }
            }
#ifdef DEBUG_MSG_CONSISTENCY
            platform->debug_print("Balise error. Telegram not accepted. accepted1="+std::to_string(accepted1)+", accepted2="+std::to_string(accepted2));
#endif
            if (supervising_rbc)
                supervising_rbc->report_error(2);
            trigger_reaction(1);
        }
        return;
    }
    
    if (dir == -1 && reading_bg_link)
        dir = passed_dir = orientated_dir = reading_bg_link->reverse_dir;
    if (dir == -1 && orientation == -1)
        bg_reference.orientation = 0;
    
    position_update_bg_passed({nid_c, nid_bg}, linked, bg_reference, dir);

    handle_telegrams(message, bg_reference, dir, timestamp, {nid_c, nid_bg}, linked, m_version);
    if (dir != -1)
        geographical_position_handle_bg_passed({nid_c, nid_bg}, bg_reference, dir == 1);
}
bool info_compare(const std::shared_ptr<etcs_information> &i1, const std::shared_ptr<etcs_information> &i2)
{
    //  Infill information shall be evaluated considering all other non-infill information in that message
    if (i1->infill.has_value() != i2->infill.has_value())
        return !i1->infill;
    // Other information shall be evaluated considering the level transition information
    if ((i1->index_level == 8 || i1->index_level == 9) != (i2->index_level == 8 || i2->index_level == 9))
        return i1->index_level == 8 || i1->index_level == 9;
    // Linking information shall be evaluated prior to any other location related information
    if ((i1->index_level == 1) != (i2->index_level == 1))
        return i1->index_level == 1;
    // Process MA after route info has been accepted
    if ((i2->index_level == 3) != (i1->index_level == 3))
        return i2->index_level == 3;
    return false;
}
void handle_information_set(std::list<std::shared_ptr<etcs_information>> &ordered_info, bool from_buffer)
{
    if (ordered_info.empty()) {
        relocate();
        return;
    }
    if (!from_buffer) {
        if (!ongoing_transition)
            transition_buffer.clear();
        else
            transition_buffer.push_back({});
    }
    ordered_info.sort(info_compare);
    bool infill = false;
    optional<distance> location;
    bool relocated = false;
    for (auto it = ordered_info.begin(); ;)
    {
        if (!relocated && (it == ordered_info.end() || ((*it)->index_level != 1 && (*it)->index_level != 8 && (*it)->index_level != 9) || ((*it)->infill && !infill))) {
            relocate();
            relocated = true;
            location = get_reference_location(infill ? *ordered_info.back()->infill : ordered_info.front()->nid_bg, infill || ordered_info.front()->is_linked_bg, !infill);
            // Distance part of level transition has to be handled after linking
            if (ongoing_transition && !ongoing_transition->ref_loc && !ongoing_transition->immediate) {
                if (!location)
                    ongoing_transition = {};
                else
                    ongoing_transition->ref_loc = location;
            }
        }
        if (it != ordered_info.end() && (*it)->infill && !infill) {
            infill = true;
            location = {};
            relocated = false;
            continue;
        }
        if (it != ordered_info.end())
        {
            if (from_buffer)
                (*it)->reevaluated = true;
            if (location && !infill)
                (*it)->ref = *location + (*it)->shift;
            else
                (*it)->ref = location;
            try_handle_information(*it, ordered_info);
            ++it;
            continue;
        }
        if (it == ordered_info.end())
            break;
    }
    calculate_SvL();
    if (!transition_buffer.empty()) {
        if (transition_buffer.back().empty()) {
            transition_buffer.pop_back();
        } else {
            for (auto &it : orbgs) {
                if (it.first.nid_lrbg == transition_buffer.back().front()->nid_bg)
                    it.second |= 2;
            }
        }
    }
    if (transition_buffer.size() > 3)
        transition_buffer.pop_front();
}
void handle_telegrams(std::vector<eurobalise_telegram> message, dist_base dist, int dir, int64_t timestamp, bg_id nid_bg, bool linked, int m_version)
{
    if (NV_NID_Cs.find(nid_bg.NID_C) == NV_NID_Cs.end()) {
        reset_national_values();
        operate_version(m_version, false);
    }
    if (VERSION_X(m_version) > VERSION_X(operated_version))
        operate_version(m_version, false);
    std::set<virtual_balise_cover> old_vbcs = vbcs;
    for (auto it = old_vbcs.begin(); it != old_vbcs.end(); ++it) {
        if (it->NID_C != nid_bg.NID_C)
            remove_vbc(*it);
    }

    std::list<std::shared_ptr<etcs_information>> ordered_info;
    for (int i=0; i<message.size(); i++) {
        eurobalise_telegram t = message[i];
        optional<bg_id> infill;
        for (int j=0; j<t.packets.size(); j++) {
            ETCS_packet *p = t.packets[j].get();
            if (p->directional) {
                auto *dp = (ETCS_directional_packet*)p;
                if ((dir == -1 && dp->Q_DIR != dp->Q_DIR.Both) || (dp->Q_DIR == dp->Q_DIR.Nominal && dir == 1) || (dp->Q_DIR == dp->Q_DIR.Reverse && dir == 0)) {
#ifdef DEBUG_MSG_CONSISTENCY
                    if (dir == -1)
                        platform->debug_print("Directional packet rejected due to unknown BG direction");
#endif
                    continue;
                }
            }
            if (p->NID_PACKET == 136) {
                InfillLocationReference ilr = *((InfillLocationReference*)p);
                infill = bg_id({ilr.Q_NEWCOUNTRY == ilr.Q_NEWCOUNTRY.SameCountry ? nid_bg.NID_C : ilr.NID_C, (int)ilr.NID_BG});
            } else if (p->NID_PACKET == 80 || p->NID_PACKET == 49 || p->NID_PACKET == 181) {
                for (auto it = ordered_info.rbegin(); it!=ordered_info.rend(); ++it) {
                    if (it->get()->index_level == 3 || it->get()->index_level == 39) {
                        it->get()->linked_packets.push_back(t.packets[j]);
                        break;
                    }
                }
            }
            std::vector<etcs_information*> info = construct_information(p, nullptr);
            for (int i=0; i<info.size(); i++) {
                info[i]->linked_packets.push_back(t.packets[j]);
                info[i]->infill = infill;
                info[i]->dir = dir;
                info[i]->fromRBC = nullptr;
                info[i]->timestamp = timestamp;
                info[i]->nid_bg = nid_bg;
                info[i]->is_linked_bg = linked;
                info[i]->version = m_version;
                ordered_info.push_back(std::shared_ptr<etcs_information>(info[i]));
            }
        }
    }
    handle_information_set(ordered_info);
}
bool handle_radio_message(std::shared_ptr<euroradio_message> message, communication_session *session)
{
    message = translate_message(message, session->version);
    std::list<std::shared_ptr<etcs_information>> ordered_info;
    bg_id lrbg = message->NID_LRBG.get_value();
    bool valid_lrbg = false;
    int dir = -1;
    double shift = 0;
    optional<dist_base> pos;
    for (auto &it : orbgs) {
        if (it.first.nid_lrbg == lrbg) {
            pos = it.first.position;
            if (mode == Mode::SH || mode == Mode::PS || mode == Mode::SL)
                dir = odometer_direction * it.first.position.orientation == 1 ? it.first.dir : 1-it.first.dir;
            else
                dir = odometer_orientation * it.first.position.orientation == 1 ? it.first.dir : 1-it.first.dir;
            break;
        }
    }
    if (!pos && (message->NID_LRBG!=message->NID_LRBG.Unknown || !session->accept_unknown_position)) {
#ifdef DEBUG_MSG_CONSISTENCY
        platform->debug_print("Radio message rejected: unknown LRBG");
#endif
        session->report_error(3);
        return false;
    }
    switch (message->NID_MESSAGE) {
        case 15: {
            auto *emerg = (conditional_emergency_stop*)message.get();
            if ((dir == -1 && emerg->Q_DIR != emerg->Q_DIR.Both) || (emerg->Q_DIR == emerg->Q_DIR.Nominal && dir == 1) || (emerg->Q_DIR == emerg->Q_DIR.Reverse && dir == 0))
                return false;
            shift = emerg->D_REF.get_value(emerg->Q_SCALE) * (dir == 1 ? -1 : 1);
            break;
        }
        case 33: {
            auto *ma = (MA_shifted_message*)message.get();
            shift = ma->D_REF.get_value(ma->Q_SCALE) * (dir == 1 ? -1 : 1);
            break;
        }
        case 34: {
            auto *taf = (taf_request_message*)message.get();
            if ((dir == -1 && taf->Q_DIR != taf->Q_DIR.Both) || (taf->Q_DIR == taf->Q_DIR.Nominal && dir == 1) || (taf->Q_DIR == taf->Q_DIR.Reverse && dir == 0))
                return false;
            shift = taf->D_REF.get_value(taf->Q_SCALE) * (dir == 1 ? -1 : 1);
            break;
        }
        default:
            break;
    }
    {
        etcs_information* info = nullptr;
        switch (message->NID_MESSAGE) {
            case 2:
                info = new SR_authorisation_info();
                break;
            case 6:
                info = new trip_exit_acknowledge_information();
                break;
            case 8:
                info = new etcs_information(38, 40, [session]() {
                    session->train_data_ack_pending = false;
                });
                info->location_based = false;
                break;
            case 15:{
                auto *emerg = (conditional_emergency_stop*)message.get();
                if (pos && !((emerg->Q_DIR == emerg->Q_DIR.Nominal && dir == 1) && (emerg->Q_DIR == emerg->Q_DIR.Reverse && dir == 0))) {
                    
                    distance d = distance::from_odometer(d_estfront);
                    info = new ces_information(d);
                }
                break;
            }
            case 16:
                info = new etcs_information(40, 42, [message]() {
                    auto *emerg = (unconditional_emergency_stop*)message.get();
                    handle_unconditional_emergency_stop(emerg->NID_EM);
                    emergency_acknowledgement_message *ack = new emergency_acknowledgement_message();
                    ack->NID_EM = emerg->NID_EM;
                    ack->Q_EMERGENCYSTOP.rawdata = 2;
                    supervising_rbc->queue(std::shared_ptr<euroradio_message_traintotrack>(ack));
                });
                info->location_based = false;
                break;
            case 18:
                info = new etcs_information(42, 44, [message]() {
                    auto *emerg = (emergency_stop_revocation*)message.get();
                    revoke_emergency_stop(emerg->NID_EM);
                });
                info->location_based = false;
                break;
            case 27:
                info = new etcs_information(43, 45, []() {
                    update_dialog_step("SH refused", "");
                });
                info->location_based = false;
                break;
            case 28:
                info = new SH_authorisation_info();
                break;
            case 34:{
                auto *taf = (taf_request_message*)message.get();
                if (!((taf->Q_DIR == taf->Q_DIR.Nominal && dir == 1) && (taf->Q_DIR == taf->Q_DIR.Reverse && dir == 0))) {
                    info = new taf_request_information();
                }
                break;
            }
            case 40:
                info = new etcs_information(50, 52, []() {
                    if (som_status == D33 || som_status == D22)
                        som_status = A38;
                });
                info->location_based = false;
                break;
            case 41:
                info = new etcs_information(51, 53, []() {
                    if (som_status == D33 || som_status == D22)
                        som_status = A23;
                });
                info->location_based = false;
                break;
            case 43:
                info = new etcs_information(52, 54, [](){ position_valid = true; });
                info->location_based = false;
                break;
            case 45:
                info = new coordinate_system_information();
                info->location_based = false;
                break;
            default:
                break;
        }
        if (info != nullptr) {
            info->dir = dir;
            info->fromRBC = session->isRBC ? session : nullptr;
            info->nid_bg = lrbg;
            info->shift = shift;
            info->infill = {};
            info->timestamp = message->T_TRAIN.get_value();
            info->message = message;
            info->version = session->version;
            ordered_info.push_back(std::shared_ptr<etcs_information>(info));
        }
    }
    optional<bg_id> infill;
    for (int j=0; j<message->packets.size(); j++) {
        ETCS_packet *p = message->packets[j].get();
        if (p->directional) {
            auto *dp = (ETCS_directional_packet*)p;
            if ((dir == -1 && dp->Q_DIR != dp->Q_DIR.Both) || (dp->Q_DIR == dp->Q_DIR.Nominal && dir == 1) || (dp->Q_DIR == dp->Q_DIR.Reverse && dir == 0)) {
#ifdef DEBUG_MSG_CONSISTENCY
                if (dir == -1)
                    platform->debug_print("Directional packet rejected due to unknown LRBG direction");
#endif
                continue;
            }
        }
        if (p->NID_PACKET == 136) {
            InfillLocationReference ilr = *((InfillLocationReference*)p);
            infill = bg_id({ilr.Q_NEWCOUNTRY == ilr.Q_NEWCOUNTRY.SameCountry ? lrbg.NID_C : ilr.NID_C, (int)ilr.NID_BG});
        } else if (p->NID_PACKET == 80) {
            for (auto it = ordered_info.rbegin(); it!=ordered_info.rend(); ++it) {
                if (it->get()->index_level == 3 || it->get()->index_level == 39) {
                    it->get()->linked_packets.push_back(message->packets[j]);
                    break;
                }
            }
        } else if (p->NID_PACKET == 49) {
            for (auto it = ordered_info.rbegin(); it!=ordered_info.rend(); ++it) {
                if (it->get()->index_level == 3 || it->get()->index_level == 39 || it->get()->index_level == 44) {
                    it->get()->linked_packets.push_back(message->packets[j]);
                    break;
                }
            }
        } else if (p->NID_PACKET == 63) {
            for (auto it = ordered_info.begin(); it!=ordered_info.end(); ++it) {
                if (it->get()->index_level == 14) {
                    it->get()->linked_packets.push_back(message->packets[j]);
                    break;
                } 
            }
        }
        std::vector<etcs_information*> info = construct_information(p, message.get());
        for (int i=0; i<info.size(); i++) {
            info[i]->linked_packets.push_back(message->packets[j]);
            info[i]->dir = dir;
            info[i]->fromRBC = session->isRBC ? session : nullptr;
            info[i]->nid_bg = lrbg;
            info[i]->shift = shift;
            info[i]->infill = infill;
            info[i]->timestamp = message->T_TRAIN.get_value()*10;
            info[i]->message = message;
            info[i]->version = session->version;
            ordered_info.push_back(std::shared_ptr<etcs_information>(info[i]));
        }
    }
    if (pos) {
        session->accept_unknown_position = false;
    } else {
        for (auto &i : ordered_info) {
            if (i->location_based || i->index_level == 1) {
#ifdef DEBUG_MSG_CONSISTENCY
                platform->debug_print("Radio message rejected: location-based information sent with unknown LRBG");
#endif
                session->report_error(3);
                return false;
            }
        }
    }
    handle_information_set(ordered_info);
    return true;
}
struct level_filter_data
{
    int num;
    Level level;
    bool fromRBC;
    bool operator<(const level_filter_data &o) const
    {
        if (num==o.num) {
            if (level == o.level)
                return fromRBC<o.fromRBC;
            return level<o.level;
        }
        return num<o.num;
    }
};
struct accepted_condition
{
    bool reject;
    std::set<int> exceptions;
};
std::map<level_filter_data, accepted_condition> level_filter_index;
bool level_filter(std::shared_ptr<etcs_information> info, const std::list<std::shared_ptr<etcs_information>> &message) 
{
    if (info->infill && ((level != Level::N1 && (!ongoing_transition || ongoing_transition->leveldata.level != Level::N1 || (level != Level::N2 && level != Level::N3))) || (mode != Mode::FS && mode != Mode::OS)))
        return false;
    accepted_condition s = level_filter_index[{info->index_level, level, info->fromRBC != nullptr}];
    if (!s.reject && info->infill)
    {
        accepted_condition s2 = level_filter_index[{32, level, info->fromRBC != nullptr}];
        if (s2.reject) s = s2;
    }
    if (!s.reject) {
        if (s.exceptions.find(3) != s.exceptions.end()) {
            if (supervising_rbc && supervising_rbc->train_data_ack_pending)
                return false;
        }
        if (s.exceptions.find(4) != s.exceptions.end()) {
            if (info->linked_packets.begin()->get()->NID_PACKET == 12) {
                Level1_MA ma = *((Level1_MA*)info->linked_packets.begin()->get());
                movement_authority MA = movement_authority(*info->ref, ma, info->timestamp);
                dist_base end = MA.get_abs_end().max;
                dist_base ssp_start = SSP_begin();
                dist_base ssp_end = SSP_end();
                if (ssp_end<end || ssp_start > d_estfront)
                    return false;
                if (get_gradient().empty() || (--get_gradient().end())->first<end || get_gradient().begin()->first > d_estfront)
                    return false;   
            } else if (info->linked_packets.begin()->get()->NID_PACKET == 15) {
                Level2_3_MA ma = *((Level2_3_MA*)info->linked_packets.begin()->get());
                movement_authority MA = movement_authority(*info->ref, ma, info->timestamp);
                dist_base end = MA.get_abs_end().max;
                dist_base ssp_start = SSP_begin();
                dist_base ssp_end = SSP_end();
                if (ssp_end<end || ssp_start > d_estfront)
                    return false;
                if (get_gradient().empty() || (--get_gradient().end())->first<end || get_gradient().begin()->first > d_estfront)
                    return false;   
            }
        }
        if (s.exceptions.find(5) != s.exceptions.end()) {
            if (!emergency_stops.empty())
                return false;
        }
        if (s.exceptions.find(8) != s.exceptions.end()) {
            TemporarySpeedRestriction tsr = *((TemporarySpeedRestriction*)info->linked_packets.begin()->get());
            if(tsr.NID_TSR != tsr.NID_TSR.NonRevocable && inhibit_revocable_tsr) return false;
        }
        if (s.exceptions.find(9) != s.exceptions.end()) {
            if (!ongoing_transition || (ongoing_transition->leveldata.level != Level::N2 && ongoing_transition->leveldata.level != Level::N3))
                return false;
        }
        if (s.exceptions.find(10) != s.exceptions.end()) {
            auto &msg = *((coordinate_system_assignment*)info->message->get());
            bg_id prvlrbg = {-1,-1};
            bg_id memorized_lrbg = prvlrbg;
            for (auto &it : orbgs) {
                if ((it.second&1) == 0) {
                    if (it.first.nid_lrbg == msg.NID_LRBG.get_value() && prvlrbg.NID_BG >= 0) {
                        if (memorized_lrbg.NID_BG >= 0 && memorized_lrbg != prvlrbg)
                            return false;
                        else
                            memorized_lrbg = prvlrbg;
                    }
                    prvlrbg = it.first.nid_lrbg;
                }
            }
        }
        if (s.exceptions.find(11) != s.exceptions.end()) {
            if (ongoing_transition)
                return false;
            for (auto m : message) {
                if (m->index_level == 8)
                    return false;
            }
        }
        if (s.exceptions.find(13) != s.exceptions.end()) {
            bool ltr_order_received = false;
            for (auto m : message) {
                if (m->index_level == 8) {
                    LevelTransitionOrder LTO = *(LevelTransitionOrder*)m->linked_packets.front().get();
                    Level lv = level_transition_information(LTO).leveldata.level;
                    if (lv == Level::N1 || lv == Level::N2 || level == Level::N3)
                        ltr_order_received = true;
                } else if (m->index_level == 9) {
                    ConditionalLevelTransitionOrder CLTO = *(ConditionalLevelTransitionOrder*)m->linked_packets.front().get();
                    Level lv = level_transition_information(CLTO).leveldata.level;
                    if (lv == Level::N1 || lv == Level::N2 || level == Level::N3)
                        ltr_order_received = true;
                }
            }
            if (!ltr_order_received)
                return false;
        }
        if (s.exceptions.find(14) != s.exceptions.end()) {
            SessionManagement &session = *(SessionManagement*)info->linked_packets.front().get();
            contact_info info = {session.NID_C, session.NID_RBC, session.NID_RADIO};
            if (session.Q_RBC == session.Q_RBC.EstablishSession) {
                if (accepting_rbc && accepting_rbc->contact == info)
                    return false;
                for (auto m : message) {
                    if (m->index_level == 16) {
                        RBCTransitionOrder o = *(RBCTransitionOrder*)m->linked_packets.front().get();
                        contact_info info2 = {o.NID_C, o.NID_RBC, o.NID_RADIO};
                        if (info2 == info)
                            return false;
                    }
                }
            }
            return true;
        }
        return true;
    } else {
        if (s.exceptions.find(1) != s.exceptions.end()) {
            if (ongoing_transition && ongoing_transition->leveldata.level == Level::N1)
                transition_buffer.back().push_back(info);
            return false;
        }
        if (s.exceptions.find(2) != s.exceptions.end()) {
            if (ongoing_transition && (ongoing_transition->leveldata.level == Level::N2 || ongoing_transition->leveldata.level == Level::N3))
                transition_buffer.back().push_back(info);
            return false;
        }
        return false;
    }
    return false;
}
void set_level_filter()
{
    std::vector<std::vector<std::string>> conds = {
        {"A","A","A","A","A","R2","R2","R2","A","A"},
        {"R1","R1","A","R1","R1","R2","R2","R2","A3","A3"},
        {"R1","R1","A","R1","R1","","","","",""},
        {"R1","R1","A4","R1","R1","R2","R2","R2","A3,4,5","A3,4,5"},
        {"R","R","A","R","R","","","","",""},
        {"R1","R1","A","R1","R1","R2","R2","R2","A3","A3"},
        {"R1","R1","A","R1","R1","R2","R2","R2","A3","A3"},
        {"R1","R1","A","R1","R1","R2","R2","R2","A3","A3"},
        {"A","A","A","A","A","A","A","A","A","A"},
        {"A11","A11","A11","A11","A11","","","","",""},
        {"A","A","A","A14","A14","A","A","A","A","A"},
        {"A","A","A","A","A","A","A","A","A","A"},
        {"","","","","","A","A","A","A","A"},
        {"","","","","","A","A","A","A","A"},
        {"","","","","","R","R","R","A3","A3"},
        {"R","R","A","A","A","","","","",""},
        {"R","R","A","R","R","","","","",""},
        {"A","R1,2","A","A8","A8","R2","R2","R2","A3","A3"},
        {"A","R1,2","A","A","A","R2","R2","R2","A3","A3"},
        {"","","","","","R2","R2","R2","A","A"},
        {"A","R1,2","A","A","A","","","","",""},
        {"R1","R1","A","R1","R1","R2","R2","R2","A3","A3"},
        {"R1","R1","A","R","R","R2","R2","R2","A","A"},
        {"A","R1,2","A","A","A","R2","R2","R2","A12","A12"},
        {"A","R1,2","A","A","A","R2","R2","R2","A12","A12"},
        {"A","R1,2","A","A","A","R2","R2","R2","A","A"},
        {"R","R","R","A","A","R","R","R","A3","A3"},
        {"A13","A13","A","A","A","","","","",""},
        {"A","A","A","A","A","","","","",""},
        {"R","R","A","R1","R1","","","","",""},
        {"R","R","A","R","R","","","","",""},
        {"A","A","A","A","A","","","","",""},
        {"","","","","","A10","A10","A10","A10","A10"},
        {"R","R","A","R1","R1","","","","",""},
        {"R1","R1","A","R1","R1","R2","R2","R2","A3","A3"},
        {"A","A","A","A","A","","","","",""},
        {"A","A","A","A","A","","","","",""},
        {"","","","","","R","R","R","A","A"},
        {"","","","","","A","A","A","A","A"},
        {"","","","","","R","R","R","A3,4,5","A3,4,5"},
        {"","","","","","R2","R2","R2","A","A"},
        {"","","","","","R2","R2","R2","A","A"},
        {"","","","","","R","R","R","A","A"},
        {"","","","","","R","R","R","A3","A3"},
        {"","","","","","R","R","R","A3","A3"},
        {"A","A","A","A","A","A","A","A","A","A"},
        {"A","A","A","A","A","","","","",""},
        {"","","","","","R","R","R","A3","A3"},
        {"","","","","","R","R","R","A","A"},
        {"A","A","A","A","A","A","A","A","A","A"},
        {"A","A","A","A","A","","","","",""},
        {"","","","","","R","R","R","A","A"},
        {"","","","","","R","R","R","A","A"},
        {"R1","R1","A","R1","R1","R2","R2","R2","A3","A3"},
        {"R1","R1","A","R1","R1","R2","R2","R2","A3","A3"},
        {"A","A","A","A","A","","","","",""},
        {"A9","A9","A9","R","R","","","","",""},
        {"R1","R1","A","R1","R1","R2","R2","R2","A3","A3"},
        {"R1,2","R1,2","A","A","A","R2","R2","R2","A3","A3"},
        {"A","A","A","A","A","","","","",""},
        {"A","A","A","A","A","","","","",""},
        {"R1","R1","A","R1","R1","R2","R2","R2","A3,5","A3,5"},
        {"R","R","A","R","R","R","R","R","A","A"},
        {"A","A","A","A","A","A","A","A","A","A"}
    };
    Level levels[] = {Level::N0, Level::NTC, Level::N1, Level::N2, Level::N3};
    for (int i=0; i<conds.size(); i++) {
        for (int j=0; j<10; j++) {
            std::string str = conds[i][j];
            if (str != "") {
                bool rej = str[0] == 'R';
                str = str.substr(1);
                std::set<int> except;
                for(;!str.empty();) {
                    std::size_t index = str.find_first_of(',');
                    except.insert(std::stoi(str.substr(0, index)));
                    if (index != std::string::npos)
                        str = str.substr(index+1);
                    else
                        break;
                }
                level_filter_index[{i, levels[j%5], j>4}] = {rej, except};
            }
        }
    }
}
struct mode_filter_data
{
    int num;
    Mode mode;
    bool operator<(const mode_filter_data &o) const
    {
        if (num==o.num) {
            return mode<o.mode;
        }
        return num<o.num;
    }
};
std::map<mode_filter_data, accepted_condition> mode_filter_index;
void set_mode_filter()
{
    std::vector<std::vector<std::string>> conds = {
        {"NR","A2","A","A","A","A","A","A","A","A","A","A","A1","NR","NR","A","A"},
        {"NR","A2,4","R","R","A","A","A","A","R","A","A","R","A1","NR","NR","A","R"},
        {"NR","A2,4","R","R","A","A","A","A","R","R","A","R","A1","NR","NR","A","R"},
        {"NR","A2,4","R","R","A","A","A","A","R","R","A","R","A1","NR","NR","A","R"},
        {"NR","R","R","R","A","A","R","A","R","R","R","R","R","NR","NR","R","R"},
        {"NR","A2,4","R","R","A","A","A","A","R","R","A","R","A1","NR","NR","A","R"},
        {"NR","A2,4","R","R","A","A","A","A","R","R","A","R","A1","NR","NR","A","R"},
        {"NR","A2,4","R","R","A","A","A","A","R","R","A","R","A1","NR","NR","A","R"},
        {"NR","A2","R","R","A","A","A","A","R","R","A","A","A1","NR","NR","A","R"},
        {"NR","A2","R","R","A","A","A","A","R","R","A","A","A1","NR","NR","R","R"},
        {"NR","A2","A7","A7","A","A","A","A","A","A","A","A","A1,5","NR","NR","A","R"},
        {"NR","A","A3","A3","A","A","A","A","A","A","A","A","A1","NR","NR","A","A"},
        {"NR","A2","A","A","A","A","A","A","A","A","A","A","A1","NR","NR","A","A"},
        {"NR","A2","R","R","A","A","A","A","R","R","A","R","A1","NR","NR","A","R"},
        {"NR","A2","R","R","A","A","A","A","R","A","A","R","A1","NR","NR","A","A"},
        {"NR","A2,4","R","R","R","R","A","R","R","R","R","R","A1","NR","NR","R","R"},
        {"NR","R","R","R","R","R","A","R","R","R","R","R","R","NR","NR","R","R"},
        {"NR","R","R","R","R","R","A6","R","R","R","R","R","R","NR","NR","R","R"},
        {"NR","A2,4","R","R","A","A","A","A","R","R","A","A","A1","NR","NR","A","R"},
        {"NR","A2,4","R","R","A","A","A","A","R","R","A","A","A1","NR","NR","A","R"},
        {"NR","A2","R","R","A","A","A","A","R","R","A","A","A1","NR","NR","A","R"},
        {"NR","A2,4","R","R","A","A","A","A","R","R","A","A","A1","NR","NR","A","R"},
        {"NR","A2,4","R","R","A","A","A","A","R","R","A","R","A1","NR","NR","A","R"},
        {"NR","A2,4","R","R","A","A","A","A","R","R","A","R","A1","NR","NR","A","R"},
        {"NR","A2","R","R","A","A","A","A","R","R","A","A","A1","NR","NR","A","A"},
        {"NR","A2","R","R","A","A","A","A","R","R","A","A","A1","NR","NR","A","A"},
        {"NR","A2","R","R","A","A","A","A","R","A","A","A","A1","NR","NR","A","R"},
        {"NR","A2,4","A8","A8","A","A","A","A","A","A","R","A","A1","NR","NR","R","R"},
        {"NR","R","R","A","R","R","R","R","R","R","R","R","R","NR","NR","R","R"},
        {"NR","R","A","R","R","R","R","R","R","R","R","R","R","NR","NR","R","R"},
        {"NR","R","R","R","A","A","A","A", "R", "R", "R", "R", "R","NR","NR","R","R"},
        {"NR","R","R","R","A","A","A","A", "R", "R", "R", "R", "R","NR","NR","R","R"},
        {"NR","R","R","A","A","A","A","A","A","A","A","A","R","NR","NR","A","A"},
        {"NR","A2","R","R","R","R","A","R","R","A","A","R","A1","NR","NR","A","R"},
        {"NR","R","R","R","A","A","R","R","R","R","R","R","R","NR","NR","R","R"},
        {"NR","A2,4","R","R","A","A","A","A","R","A","A","A","A1","NR","NR","A","R"},
        {"NR","A2,4","R","R","A","A","A","A","R","R","A","R","A1","NR","NR","A","R"},
        {"NR","A2,4","A","A","A","A","A","A","A","A","A","A","A1","NR","NR","A","R"},
        {"NR","A2","A","A","A","A","A","A","A","A","A","A","A","NR","NR","A","A"},
        {"NR","R","R","R","R","R","R","R","R","R","R","R","A","NR","NR","R","R"},
        {"NR","A2","R","R","A","A","A","A","R","R","A","A","A","NR","NR","A","A"},
        {"NR","R","R","R","A","A","R","A","R","R","R","R","R","NR","NR","R","R"},
        {"NR","A2","R","R","A","A","A","A","R","R","A","R","R","NR","NR","A","R"},
        {"NR","R","R","R","A","A","R","A","R","R","A","R","R","NR","NR","A","R"},
        {"NR","R","R","R","A","A","R","A","R","R","R","R","A1","NR","NR","R","R"},
        {"NR","A2","R","R","A","A","A","A","R","R","R","R","A1","NR","NR","R","R"},
        {"NR","A2","R","R","A","A","A","A","R","R","R","R","A1","NR","NR","R","R"},
        {"NR","A","A","A","A","A","A","A","A","A","A","A","A","NR","NR","A","A"},
        {"NR","A","A","A","A","A","A","A","A","A","A","A","A","NR","NR","A","A"},
        {"NR","A2","R","R","R","A","A","A","R","R","R","R","A1","NR","NR","R","R"},
        {"NR","A2","R","R","A","A","A","A","R","A","R","A","A","NR","NR","R","A"},
        {"NR","A","A","A","A","A","A","A","A","A","A","A","A","NR","NR","A","A"},
        {"NR","A2","R","R","R","R","R","R","R","R","R","R","R","NR","NR","R","R"},
        {"NR","A2","R","R","R","R","R","R","R","R","R","R","R","NR","NR","R","R"},
        {"NR","A2","R","R","R","R","R","R","R","R","R","R","R","NR","NR","R","R"},
        {"NR","A2,4","R","R","A","A","A","A","R","R","A","R","A1","NR","NR","A","A"},
        {"NR","A2,4","R","R","A","A","A","A","R","R","A","R","A1","NR","NR","A","A"},
        {"NR","A2","A","A","A","A","A","A","A","A","A","A","A","NR","NR","A","A"},
        {"NR","A2","R","R","A","A","A","A","R","R","A","A","A","NR","NR","A","R"},
        {"NR","A2,4","R","R","A","A","A","A","R","R","A","R","A1","NR","NR","A","R"},
        {"NR","A2,4","R","R","A","A","A","A","R","R","A","R","A1","NR","NR","A","R"},
        {"NR","A","A","A","A","A","A","A","A","A","A","A","A","NR","NR","A","A"},
        {"NR","A","A","A","A","A","A","A","A","A","A","A","A","NR","NR","A","A"},
        {"NR","R","R","R","A9","A","A9","A9","R","R","A9","R","R","NR","NR","A9","R"},
        {"NR","R","R","R","R","A","R","R","R","R","R","R","R","NR","NR","R","R"},
        {"NR","A","A","A","A","A","A","A","A","A","A","A","A","NR","NR","A","A"}
    };
    Mode modes[] = {Mode::NP, Mode::SB, Mode::PS, Mode::SH, Mode::FS, Mode::LS, Mode::SR, Mode::OS, Mode::SL, Mode::NL, Mode::UN, Mode::TR, Mode::PT, Mode::SF, Mode::IS, Mode::SN, Mode::RV};
    for (int i=0; i<conds.size(); i++) {
        for (int j=0; j<17; j++) {
            std::string str = conds[i][j];
            if (str != "" && str != "NR") {
                bool rej = str[0] == 'R';
                str = str.substr(1);
                std::set<int> except;
                for(;!str.empty();) {
                    std::size_t index = str.find_first_of(',');
                    except.insert(std::stoi(str.substr(0, index)));
                    if (index != std::string::npos)
                        str = str.substr(index+1);
                    else
                        break;
                }
                mode_filter_index[{i, modes[j]}] = {rej, except};
            }
        }
    }
}
void set_message_filters()
{
    set_mode_filter();
    set_level_filter();
}
bool second_filter(std::shared_ptr<etcs_information> info, const std::list<std::shared_ptr<etcs_information>> &message)
{
    if (!info->fromRBC || info->fromRBC == supervising_rbc)
        return true;
    if (info->fromRBC == handing_over_rbc)
        return info->index_level == 10;
    if (info->index_level == 42 || info->index_level == 39 || info->index_level == 61 || info->index_level == 62)
        return false;
    if (info->index_level == 10 || info->index_level == 37)
        return true;
    transition_buffer.back().push_back(info);
    return false;
}
bool mode_filter(std::shared_ptr<etcs_information> info, const std::list<std::shared_ptr<etcs_information>> &message)
{
    if (info->infill && mode != Mode::FS && mode != Mode::LS)
        return false;
    accepted_condition s = mode_filter_index[{info->index_mode, mode}];
    if (s.reject) {
        return false;
    } else {
        if (s.exceptions.find(1) != s.exceptions.end()) {
            if (level == Level::N1 || !trip_exit_acknowledged || info->timestamp < trip_exit_acknowledge_timestamp) return false;
        }
        if (s.exceptions.find(2) != s.exceptions.end()) {
            if (!cab_active[0] && !cab_active[1]) return false;
        }
        if (s.exceptions.find(4) != s.exceptions.end()) {
            if (!train_data_valid) return false;
        }
        if (s.exceptions.find(5) != s.exceptions.end()) {
            if (info->index_level == 8) {
                LevelTransitionOrder &LTO = *(LevelTransitionOrder*)info->linked_packets.front().get();
                if (LTO.D_LEVELTR == LTO.D_LEVELTR.Now) return false;
            }
            if (info->index_level == 9)
                return false;
        }
        if (s.exceptions.find(6) != s.exceptions.end()) {
            if (overrideProcedure) return false;
        }
        if (s.exceptions.find(7) != s.exceptions.end()) {
            if (info->index_level == 8) {
                LevelTransitionOrder &LTO = *(LevelTransitionOrder*)info->linked_packets.front().get();
                if (LTO.D_LEVELTR != LTO.D_LEVELTR.Now) return false;
            }
        }
        if (s.exceptions.find(8) != s.exceptions.end()) {
            if (info->index_level == 10) {
                RBCTransitionOrder &o = *(RBCTransitionOrder*)info->linked_packets.front().get();
                if (o.D_RBCTR != 0) return false;
            }
        }
        if (s.exceptions.find(9) != s.exceptions.end()) {
            bool inside_ls = false;
            for (auto i : message) {
                if (i->index_mode == 3 && i->ref) {
                    for (auto it = ++i->linked_packets.begin(); it != i->linked_packets.end(); ++it) {
                        if (it->get()->NID_PACKET == 80) {
                            ModeProfile &profile = *(ModeProfile*)(it->get());
                            std::vector<MP_element_packet> mps;
                            mps.push_back(profile.element);
                            mps.insert(mps.end(), profile.elements.begin(), profile.elements.end());
                            distance start = *i->ref;
                            for (auto it2 = mps.begin(); it2 != mps.end(); ++it2) {
                                start += it2->D_MAMODE.get_value(profile.Q_SCALE);
                                distance end = start+it2->L_MAMODE;
                                if (start.max < d_maxsafefront(start) && d_maxsafefront(end) < end.min && it2->M_MAMODE == it2->M_MAMODE.LS)
                                    inside_ls = true;
                            }
                        }
                    }
                }
            }
            if (!inside_ls)
                return false;
        }
        return true;
    }
}
void try_handle_information(std::shared_ptr<etcs_information> info, const std::list<std::shared_ptr<etcs_information>> &message)
{
    if (info->location_based && !info->ref) return;
    if (!level_filter(info, message)) return;
    if (!second_filter(info, message)) return;
    if (!mode_filter(info, message)) return;
    info->handle();
}
std::vector<etcs_information*> construct_information(ETCS_packet *packet, euroradio_message *msg)
{
    int packet_num = packet->NID_PACKET.rawdata;
    std::vector<etcs_information*> info;
    if (packet_num == 2) {
        info.push_back(new version_order_information());
    } else if (packet_num == 3) {
        info.push_back(new national_values_information());
    } else if (packet_num == 5) {
        info.push_back(new linking_information());
    } else if (packet_num == 6) {
        info.push_back(new vbc_order());
    } else if (packet_num == 12) {
        info.push_back(new ma_information());
        info.push_back(new signalling_information());
    } else if (packet_num == 15) {
        if (msg != nullptr && msg->NID_MESSAGE == 9)
            info.push_back(new ma_shortening_information());
        else
            info.push_back(new ma_information_lv2());
    } else if (packet_num == 16) {
        info.push_back(new repositioning_information());
    } else if (packet_num == 21) {
        info.push_back(new gradient_information());
    } else if (packet_num == 27) {
        info.push_back(new issp_information());
    } else if (packet_num == 39) {
        info.push_back(new track_condition_information());
    } else if (packet_num == 40) {
        info.push_back(new track_condition_information());
    } else if (packet_num == 41) {
        info.push_back(new leveltr_order_information());
    } else if (packet_num == 42) {
        info.push_back(new session_management_information());
    } else if (packet_num == 46) {
        info.push_back(new condleveltr_order_information());
    } else if (packet_num == 51) {
        info.push_back(new axle_load_speed_profile_information());
    } else if (packet_num == 52) {
        info.push_back(new pbd_information());
    } else if (packet_num == 57) {
        info.push_back(new ma_request_params_info());
    } else if (packet_num == 58) {
        auto *prp = new position_report_params_info();
        prp->location_based = ((PositionReportParameters*)packet)->N_ITER > 0;
        info.push_back(prp);
    } else if (packet_num == 65) {
        info.push_back(new TSR_information());
    } else if (packet_num == 66) {
        info.push_back(new TSR_revocation_information());
    } else if (packet_num == 67) {
        info.push_back(new track_condition_big_metal_information());
    } else if (packet_num == 68) {
        info.push_back(new track_condition_information());
        info.push_back(new track_condition_information2());
    } else if (packet_num == 69) {
        info.push_back(new track_condition_information());
    } else if (packet_num == 70) {
        info.push_back(new route_suitability_information());
    } else if (packet_num == 72) {
        info.push_back(new plain_text_information());
    } else if (packet_num == 76) {
        info.push_back(new fixed_text_information());
    } else if (packet_num == 79) {
        info.push_back(new geographical_position_information());
    } else if (packet_num == 88) {
        info.push_back(new level_crossing_information());
    } else if (packet_num == 90) {
        info.push_back(new taf_level23_information());
    } else if (packet_num == 131) {
        info.push_back(new rbc_transition_information());
    } else if (packet_num == 132) {
        info.push_back(new danger_for_SH_information());
    } else if (packet_num == 137) {
        info.push_back(new stop_if_in_SR_information());
    } else if (packet_num == 138) {
        info.push_back(new reversing_area_information());
    } else if (packet_num == 139) {
        info.push_back(new reversing_supervision_information());
    } else if (packet_num == 140) {
        info.push_back(new train_running_number_information());
    } else if (packet_num == 141) {
        info.push_back(new TSR_gradient_information());
    } else if (packet_num == 180) {
        auto *order = (LSSMAToggleOrder*)packet;
        if (order->Q_LSSMA == order->Q_LSSMA.ToggleOff)
            info.push_back(new lssma_display_off_information());
        else
            info.push_back(new lssma_display_on_information());
    } else if (packet_num == 181) {
        info.push_back(new generic_ls_marker_information());
    } else if (packet_num == 254) {
        info.push_back(new default_balise_information());
    }
    return info;
}
