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
#include "track_condition.h"
#include "../Supervision/targets.h"
#include "../Supervision/fixed_values.h"
#include "../Time/clock.h"
#include "../TrainSubsystems/power.h"
#include "../Supervision/conversion_model.h"
std::list<std::shared_ptr<track_condition>> track_conditions;
optional<distance> restore_initial_states_various;
optional<distance> restore_initial_states_platforms;
std::set<distance> brake_change;
std::map<track_condition*, std::vector<target>> track_condition_targets; 
void add_condition();
void update_brake_contributions()
{
    std::map<distance, int> active;
    active[std::numeric_limits<double>::lowest()] = 15;
    for (auto it = track_conditions.begin(); it != track_conditions.end(); ++it) {
        active[(*it)->start]=15;
        if ((*it)->profile) active[(*it)->end]=15;
    }
    for (auto it = active.begin(); it != active.end(); ++it) {
        distance d = it->first;
        bool reg=true;
        bool shoe=true;
        bool eddyemerg=true;
        bool eddyserv=true;
        for (auto it2 = track_conditions.begin(); it2 != track_conditions.end(); ++it2) {
            if ((*it2)->start <= d && (*it2)->end > d) {
                switch((*it2)->condition) {
                    case TrackConditions::PowerLessSectionLowerPantograph:
                    case TrackConditions::PowerLessSectionSwitchMainPowerSwitch:
                    case TrackConditions::SwitchOffRegenerativeBrake:
                        reg=false;
                        break;
                    case TrackConditions::SwitchOffMagneticShoe:
                        shoe=false;
                        break;
                    case TrackConditions::SwitchOffEddyCurrentEmergencyBrake:
                        eddyemerg=false;
                        break;
                    case TrackConditions::SwitchOffEddyCurrentServiceBrake:
                        eddyserv=false;
                        break;
                    default:
                        break;
                }
            }
        }
        it->second = (reg?8:0)+(shoe?4:0)+(eddyemerg?2:0)+(eddyserv?1:0);
    }
    active_combination = active;
    target::recalculate_all_decelerations();
}
void update_track_conditions()
{
    if (restore_initial_states_various && *restore_initial_states_various<d_minsafefront(restore_initial_states_various->get_reference())) {
        for (auto it = track_conditions.begin(); it != track_conditions.end();) {
            TrackConditions c = it->get()->condition;
            if (c == TrackConditions::SoundHorn || c == TrackConditions::NonStoppingArea || c == TrackConditions::TunnelStoppingArea ||
                c == TrackConditions::PowerLessSectionLowerPantograph || c == TrackConditions::PowerLessSectionSwitchMainPowerSwitch || 
                c == TrackConditions::RadioHole || c == TrackConditions::AirTightness || c == TrackConditions::SwitchOffRegenerativeBrake ||
                c == TrackConditions::SwitchOffEddyCurrentEmergencyBrake || c == TrackConditions::SwitchOffEddyCurrentServiceBrake || c == TrackConditions::SwitchOffMagneticShoe) {

                track_condition_targets.erase(it->get());
                it = track_conditions.erase(it);
                continue;
            }
            ++it;
        }
        restore_initial_states_various = {};
        update_brake_contributions();
    }
    if (restore_initial_states_platforms && *restore_initial_states_platforms<d_minsafefront(restore_initial_states_platforms->get_reference())) {
        for (auto it = track_conditions.begin(); it != track_conditions.end(); ) {
            TrackConditions c = it->get()->condition;
            if (c == TrackConditions::StationPlatform) {
                it = track_conditions.erase(it);
                continue;
            }
            ++it;
        }
        restore_initial_states_platforms = {};
    }
    for (auto it = track_conditions.begin(); it != track_conditions.end();) {
        track_condition *c = it->get();
        double end = c->get_end_distance_to_train();
        if (end < (c->condition == TrackConditions::BigMetalMasses ? -D_keep_information : 0) && !c->display_end) {
            c->display_end = true;
            c->end_time = get_milliseconds() + T_delete_condition * 1000;
        }
        if (c->display_end && c->end_time < get_milliseconds()) {
            track_condition_targets.erase(it->get());
            it = track_conditions.erase(it);
            continue;
        }
        if (c->condition == TrackConditions::NonStoppingArea) {
            if (track_condition_targets.find(c) == track_condition_targets.end())
            {
                std::vector<target> l;
                l.push_back(target(c->start, 0, target_class::EoA));
                l.push_back(target(c->end + L_TRAIN, 0, target_class::EoA));
                track_condition_targets[c] = l;
            }
            std::vector<target> &l = track_condition_targets[c];
            target &SBId = l[0];
            target &SBIg = l[1];
            SBId.calculate_curves();
            SBIg.calculate_curves();
            distance max = d_maxsafefront(c->start.get_reference());
            distance min = d_minsafefront(c->start.get_reference());
            if (max<SBId.d_SBI1 || min > SBIg.d_SBI1) {
                c->order = c->announce = false;
            }
            if (max > SBId.d_SBI1 && min < SBIg.d_SBI1) {
                if (max < c->start) {
                    c->announce = true;
                    c->order = false;
                } else {
                    c->announce = false;
                    c->order = true;
                }
            }
        } else if (c->condition == TrackConditions::PowerLessSectionSwitchMainPowerSwitch) {
            distance pointC = c->start - V_est * 11;
            distance max = d_maxsafefront(c->start.get_reference());
            distance min = d_minsafefront(c->start.get_reference());
            if (min > c->end) {
                c->announce = false;
                c->order = false;
            } else if (max>c->start) {
                c->announce = false;
                c->order = true;
            } else if (max > pointC) {
                c->announce = true;
                c->order = false;
            }
        } else if (c->condition == TrackConditions::PowerLessSectionLowerPantograph) {
            distance pointC = c->start - V_est * 20;
            distance max = d_maxsafefront(c->start.get_reference());
            distance min = d_minsafefront(c->start.get_reference());
            if (min > c->end) {
                c->announce = false;
                c->order = false;
            } else if (max>c->start) {
                c->announce = false;
                c->order = true;
            } else if (max > pointC) {
                c->announce = true;
                c->order = false;
            }
        } else if (c->condition == TrackConditions::SoundHorn) {
            distance pointC = c->start - V_est * T_horn;
        } else if (c->condition == TrackConditions::AirTightness) {
            distance pointC = c->start - V_est * 10;
            distance max = d_maxsafefront(c->start.get_reference());
            distance min = d_minsafefront(c->start.get_reference());
            if (min + L_TRAIN > c->end) {
                c->announce = false;
                c->order = false;
            } else if (max>c->start) {
                c->announce = false;
                c->order = true;
            } else if (max > pointC) {
                c->announce = true;
                c->order = false;
            }
        }
        ++it;
    }
}
void load_track_condition_bigmetal(TrackConditionBigMetalMasses cond, distance ref)
{
    distance first = ref + cond.element.D_TRACKCOND.get_value(cond.Q_SCALE);
    for (auto it = track_conditions.begin(); it != track_conditions.end();) {
        if (it->get()->condition == TrackConditions::BigMetalMasses && it->get()->start >= first) {
            it = track_conditions.erase(it);
            continue;
        }
        ++it;
    }
    std::vector<TC_bigmetal_element_packet> elements;
    elements.push_back(cond.element);
    elements.insert(elements.end(), cond.elements.begin(), cond.elements.end());
    distance curr = ref;
    for (auto it = elements.begin(); it != elements.end(); ++it) {
        track_condition *tc = new track_condition();
        tc->start = curr + it->D_TRACKCOND.get_value(cond.Q_SCALE);
        curr = tc->start;
        tc->end = tc->start + it->L_TRACKCOND.get_value(cond.Q_SCALE);
        tc->profile = true;
        tc->condition = TrackConditions::BigMetalMasses;
        track_conditions.push_back(std::shared_ptr<track_condition>(tc));
    }
}
void load_track_condition_various(TrackCondition cond, distance ref)
{
    if (cond.Q_TRACKINIT == Q_TRACKINIT_t::InitialState) {
        restore_initial_states_various = cond.D_TRACKINIT.get_value(cond.Q_SCALE);
        return;
    }
    distance first = ref + cond.element.D_TRACKCOND.get_value(cond.Q_SCALE);
    for (auto it = track_conditions.begin(); it != track_conditions.end();) {
        TrackConditions c = it->get()->condition;
        if ((c == TrackConditions::SoundHorn || c == TrackConditions::NonStoppingArea || c == TrackConditions::TunnelStoppingArea ||
            c == TrackConditions::PowerLessSectionLowerPantograph || c == TrackConditions::PowerLessSectionSwitchMainPowerSwitch || 
            c == TrackConditions::RadioHole || c == TrackConditions::AirTightness || c == TrackConditions::SwitchOffRegenerativeBrake ||
            c == TrackConditions::SwitchOffEddyCurrentEmergencyBrake || c == TrackConditions::SwitchOffEddyCurrentServiceBrake || c == TrackConditions::SwitchOffMagneticShoe)
            && it->get()->start >= first) {
            track_condition_targets.erase(it->get());    
            it = track_conditions.erase(it);
            continue;
        }
        ++it;
    }
    std::vector<TC_element_packet> elements;
    elements.push_back(cond.element);
    elements.insert(elements.end(), cond.elements.begin(), cond.elements.end());
    distance curr = ref;
    for (auto it = elements.begin(); it != elements.end(); ++it) {
        track_condition *tc = new track_condition();
        tc->start = curr + it->D_TRACKCOND.get_value(cond.Q_SCALE);
        curr = tc->start;
        tc->end = tc->start + it->L_TRACKCOND.get_value(cond.Q_SCALE);
        tc->profile = true;
        switch(it->M_TRACKCOND) {
            case M_TRACKCOND_t::AirTightness:
                tc->condition = TrackConditions::AirTightness;
                if (Q_airtight) {
                    tc->start_symbol = 19;
                    tc->end_symbol = 20;
                    tc->announcement_symbol = 21;
                    tc->active_symbol = 19;
                    tc->end_active_symbol = 22;
                }
                break;
            case M_TRACKCOND_t::TunnelStoppingArea:
                tc->condition = TrackConditions::TunnelStoppingArea;
                break;
            case M_TRACKCOND_t::NonStoppingArea:
                tc->condition = TrackConditions::NonStoppingArea;
                tc->start_symbol = 9;
                tc->active_symbol = 10;
                tc->announcement_symbol = 11;
                break;
            case M_TRACKCOND_t::SoundHorn:
                tc->condition = TrackConditions::SoundHorn;
                tc->start_symbol = 24;
                break;
            case M_TRACKCOND_t::PowerlessLowerPantograph:
                tc->condition = TrackConditions::PowerLessSectionLowerPantograph;
                tc->start_symbol = lower_pantograph_available ? 1 : 2;
                tc->end_symbol = raise_pantograph_available ? 2 : 3;
                tc->announcement_symbol = raise_pantograph_available ? 2 : 3;
                tc->active_symbol = 1;
                tc->end_active_symbol = raise_pantograph_available ? 4 : 5;
                break;
            case M_TRACKCOND_t::PowerlessSwitchOffPower:
                tc->condition = TrackConditions::PowerLessSectionSwitchMainPowerSwitch;
                tc->start_symbol = main_power_off_available ? 5 : 6;
                tc->end_symbol = main_power_on_available ? 7 : 8;
                tc->announcement_symbol = main_power_off_available ? 6 : 7;
                tc->active_symbol = 6;
                tc->end_active_symbol = main_power_on_available ? 8 : 9;
                break;
            case M_TRACKCOND_t::RadioHole:
                tc->condition = TrackConditions::RadioHole;
                tc->start_symbol = 11;
                break;
            case M_TRACKCOND_t::SwitchOffRegenerative:
                tc->condition = TrackConditions::SwitchOffRegenerativeBrake;
                tc->start_symbol = 16;
                break;
            case M_TRACKCOND_t::SwitchOffShoe:
                tc->condition = TrackConditions::SwitchOffMagneticShoe;
                tc->start_symbol = 12;
                break;
            case M_TRACKCOND_t::SwitchOffEddyService:
                tc->condition = TrackConditions::SwitchOffEddyCurrentServiceBrake;
                tc->start_symbol = 14;
                break;
            case M_TRACKCOND_t::SwitchOffEddyEmergency:
                tc->condition = TrackConditions::SwitchOffEddyCurrentEmergencyBrake;
                tc->start_symbol = 14;
                break;
        }
        bool exists = false;
        for (auto it = track_conditions.begin(); it != track_conditions.end(); ++it) {
            track_condition *tc2 = it->get();
            if (tc2->condition == tc->condition) {
                if (tc->start <= tc2->start && tc->end >= tc2->start)
                    exists = true;
                if (tc->start >= tc2->start && tc->start <= tc2->end)
                    exists = true;
                if (tc->start <= tc2->start && tc->end >= tc2->start)
                    tc2->start = tc->start;
                if (tc->end >= tc2->end && tc->start <= tc2->end)
                    tc2->end = tc->end;
            } 
        }
        if (!exists) {
            track_conditions.push_back(std::shared_ptr<track_condition>(tc));
        }
    }
    update_brake_contributions();
}
void load_track_condition_platforms(TrackConditionStationPlatforms cond, distance ref)
{
    if (cond.Q_TRACKINIT == Q_TRACKINIT_t::InitialState) {
        restore_initial_states_platforms = cond.D_TRACKINIT.get_value(cond.Q_SCALE);
        return;
    }
    distance first = ref + cond.element.D_TRACKCOND.get_value(cond.Q_SCALE);
    for (auto it = track_conditions.begin(); it != track_conditions.end();) {
        if (it->get()->condition == TrackConditions::StationPlatform && it->get()->start >= first) {
            it = track_conditions.erase(it);
            continue;
        }
        ++it;
    }
    std::vector<TC_station_element_packet> elements;
    elements.push_back(cond.element);
    elements.insert(elements.end(), cond.elements.begin(), cond.elements.end());
    distance curr = ref;
    for (auto it = elements.begin(); it != elements.end(); ++it) {
        track_condition_platforms *tc = new track_condition_platforms();
        tc->start = curr + it->D_TRACKCOND.get_value(cond.Q_SCALE);
        curr = tc->start;
        tc->end = tc->start + it->L_TRACKCOND.get_value(cond.Q_SCALE);
        tc->profile = true;
        tc->platform_height = it->M_PLATFORM.get_value();
        tc->right_side = it->Q_PLATFORM != Q_PLATFORM_t::LeftSide;
        tc->left_side = it->Q_PLATFORM != Q_PLATFORM_t::RightSide;
        tc->condition = TrackConditions::StationPlatform;
        track_conditions.push_back(std::shared_ptr<track_condition>(tc));
    }
}