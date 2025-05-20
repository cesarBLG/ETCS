/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "track_condition.h"
#include "../Supervision/fixed_values.h"
#include "../Time/clock.h"
#include "../TrainSubsystems/power.h"
#include "../TrainSubsystems/train_interface.h"
#include "../Supervision/conversion_model.h"
std::set<std::shared_ptr<track_condition>> track_conditions;
std::set<distance> brake_change;
void add_condition();
void update_brake_contributions()
{
    std::map<dist_base, std::pair<int,int>> active;
    std::pair<int,int> def = {regenerative_brake_available<<REGENERATIVE_AVAILABLE | eddy_brake_available<<EDDY_AVAILABLE | ep_brake_available<<EP_AVAILABLE, regenerative_brake_available<<REGENERATIVE_AVAILABLE | eddy_brake_available<<EDDY_AVAILABLE | ep_brake_available<<EP_AVAILABLE | magnetic_brake_available<<MAGNETIC_AVAILABLE};
    active[dist_base::min] = def;
    for (auto it = track_conditions.begin(); it != track_conditions.end(); ++it) {
        active[(*it)->start.max]=def;
        if ((*it)->profile) active[(*it)->end.min]=def;
    }
    for (auto it = active.begin(); it != active.end(); ++it) {
        dist_base d = it->first;
        bool reg=regenerative_brake_available;
        bool shoe=magnetic_brake_available;
        bool eddyemerg=eddy_brake_available;
        bool eddyserv=eddy_brake_available;
        for (auto it2 = track_conditions.begin(); it2 != track_conditions.end(); ++it2) {
            if ((*it2)->start.max <= d && (*it2)->end.min > d) {
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
        it->second = {reg<<REGENERATIVE_AVAILABLE | eddyserv<<EDDY_AVAILABLE | ep_brake_available<<EP_AVAILABLE, reg<<REGENERATIVE_AVAILABLE | eddyemerg<<EDDY_AVAILABLE | ep_brake_available<<EP_AVAILABLE | shoe<<MAGNETIC_AVAILABLE};
    }
    active_combination = active;
    target::recalculate_all_decelerations();
}
void update_track_conditions()
{
    for (auto it = track_conditions.begin(); it != track_conditions.end();) {
        track_condition *c = it->get();
        double end = c->get_end_distance_to_train();
        if (end < 0 && !c->end_displayed) {
            c->display_end = true;
            c->end_displayed = true;
            c->end_time = get_milliseconds() + T_delete_condition * 1000;
        }
        if (c->display_end && c->end_time < get_milliseconds()) {
            c->display_end = false;
        }
        if (end < -L_TRAIN - D_keep_information && c->end_displayed && !c->display_end) {
            it = track_conditions.erase(it);
            continue;
        }
        if (c->condition == TrackConditions::NonStoppingArea) {
            if (c->targets.empty())
            {
                c->targets.push_back(std::make_shared<target>(c->start.max, 0, target_class::EoA));
                c->targets.push_back(std::make_shared<target>(c->end.min + L_TRAIN, 0, target_class::EoA));
            }
            std::vector<std::shared_ptr<target>> &l = c->targets;
            auto &SBId = *l[0];
            auto &SBIg = *l[1];
            SBId.calculate_curves(V_est, A_est, V_ura);
            SBIg.calculate_curves(V_est, A_est, V_ura);
            dist_base max = d_maxsafefront(c->start);
            dist_base min = d_minsafefront(c->start);
            if (max<SBId.d_SBI1 || min > SBIg.d_SBI1) {
                c->order = c->announce = false;
            }
            c->announce_distance = SBId.d_SBI1 - max;
            if (max > SBId.d_SBI1 && min < SBIg.d_SBI1) {
                if (max < c->start.max) {
                    c->announce = true;
                    c->order = false;
                } else {
                    c->announce = false;
                    c->order = true;
                }
            }
        } else if (c->condition == TrackConditions::PowerLessSectionSwitchMainPowerSwitch) {
            distance pointC = c->start - std::max(V_est * T_open_mps, D_open_mps);
            dist_base max = d_maxsafefront(c->start);
            dist_base min = d_minsafefront(c->start);
            distance &pointD = c->start;
            distance &pointE = c->end;
            if (pointC.max-max < 0 && pointE.min-min > -L_TRAIN) {
                track_condition_profile_external ext;
                if (pointD.max-max > -L_TRAIN)
                    ext.start = pointD.max-max;
                ext.end = pointE.min-min;
                c->external = ext;
            } else {
                c->external = {};
            }
            c->announce_distance = pointC.max-max;
            if (min > c->end.min) {
                c->announce = false;
                c->order = false;
            } else if (max>c->start.max) {
                c->announce = false;
                c->order = true;
            } else if (max > pointC.max) {
                c->announce = true;
                c->order = false;
            }
        } else if (c->condition == TrackConditions::PowerLessSectionLowerPantograph) {
            distance pointC = c->start - std::max(V_est * T_lower_pantograph, D_lower_pantograph);
            dist_base max = d_maxsafefront(c->start);
            dist_base min = d_minsafefront(c->start);
            distance &pointD = c->start;
            distance &pointE = c->end;
            if (pointC.max-max < 0 && pointE.min-min > -L_TRAIN) {
                track_condition_profile_external ext;
                if (pointD.max-max > -L_TRAIN)
                    ext.start = pointD.max-max;
                ext.end = pointE.min-min;
                c->external = ext;
            } else {
                c->external = {};
            }
            c->announce_distance = pointC.max-max;
            if (min > c->end.min) {
                c->announce = false;
                c->order = false;
            } else if (max>c->start.max) {
                c->announce = false;
                c->order = true;
            } else if (max > pointC.max) {
                c->announce = true;
                c->order = false;
            }
        } else if (c->condition == TrackConditions::SoundHorn) {
            distance pointC = c->start - V_est * T_horn;
            distance &pointE = c->end;
            c->announce_distance = pointC.est-d_estfront;
            c->order = d_estfront > pointC.est && d_estfront < pointE.est;
        } else if (c->condition == TrackConditions::AirTightness) {
            distance pointC = c->start - V_est * 10;
            dist_base max = d_maxsafefront(c->start);
            dist_base min = d_minsafefront(c->start);
            distance &pointD = c->start;
            distance &pointE = c->end;
            if (pointC.max-max < 0 && pointE.min-min > -L_TRAIN) {
                track_condition_profile_external ext;
                if (pointD.max-max > 0)
                    ext.start = pointD.max-max;
                ext.end = pointE.min-min;
                c->external = ext;
            } else {
                c->external = {};
            }
            c->announce_distance = pointC.max-max;
            if (min - L_TRAIN > c->end.min) {
                c->announce = false;
                c->order = false;
            } else if (max>c->start.max) {
                c->announce = false;
                c->order = true;
            } else if (max > pointC.max) {
                c->announce = true;
                c->order = false;
            }
        } else if (c->condition == TrackConditions::ChangeOfTractionSystem) {
            distance pointC = c->start - std::max(V_est * T_traction_system_change, D_traction_system_change);
            dist_base max = d_maxsafefront(c->start);
            dist_base min = d_minsafefront(c->start) - L_TRAIN;
            distance &pointF = c->start;
            if (pointC.max-max < 0 && pointF.min-min > 0) {
                track_condition_profile_external ext;
                if (pointF.min-min > 0)
                    ext.start = pointF.max-max;
                c->external = ext;
            } else {
                c->external = {};
            }
            c->announce_distance = pointC.max-max;
            if (c->end_displayed) {
                c->announce = false;
                c->order = false;
            } else if (max>pointF.max) {
                c->announce = false;
                c->order = true;
            } else if (max > pointC.max) {
                c->announce = false;
                c->order = true;
            }
        } else if (c->condition == TrackConditions::ChangeOfAllowedCurrentConsumption) {
            distance pointC = c->start - std::max(V_est * T_current_consumption_change, D_current_consumption_change);
            dist_base max = d_maxsafefront(c->start);
            dist_base min = d_minsafefront(c->start) - L_TRAIN;
            distance &pointF = c->start;
            if (pointC.max-max < 0 && pointF.min-min > 0) {
                track_condition_profile_external ext;
                if (pointF.min-min > 0)
                    ext.start = pointF.max-max;
                c->external = ext;
            } else {
                c->external = {};
            }
        } else if (c->condition == TrackConditions::RadioHole) {
            distance &pointD = c->start;
            distance &pointE = c->end;
            dist_base max = d_maxsafefront(c->start);
            dist_base min = d_minsafefront(c->start) - L_TRAIN;
            c->announce_distance = pointD.max-max;
            c->order = max > pointD.max && min < pointE.min;
        } else if (c->condition == TrackConditions::SwitchOffEddyCurrentEmergencyBrake ||
            c->condition == TrackConditions::SwitchOffEddyCurrentServiceBrake ||
            c->condition == TrackConditions::SwitchOffMagneticShoe ||
            c->condition == TrackConditions::SwitchOffRegenerativeBrake) {
            distance pointC = c->start - std::max(V_est * T_brake_inhibit, D_brake_inhibit);
            dist_base max = d_maxsafefront(c->start);
            dist_base min = d_minsafefront(c->start) - L_TRAIN;
            distance &pointD = c->start;
            distance &pointE = c->end;
            if (pointC.max-max < 0 && pointE.min-min > 0) {
                track_condition_profile_external ext;
                if (pointD.max-max > 0)
                    ext.start = pointD.max-max;
                ext.end = pointE.min-min;
                c->external = ext;
            } else {
                c->external = {};
            }
            c->announce_distance = pointC.max-max;
            if (min > c->end.min) {
                c->announce = false;
                c->order = false;
            } else if (max>c->start.max) {
                c->announce = false;
                c->order = true;
            } else if (max > pointC.max) {
                c->announce = true;
                c->order = false;
            }
        } else if (c->condition == TrackConditions::StationPlatform) {
            distance pointC = c->start - std::max(V_est * T_station_platform, D_station_platform);
            dist_base max = d_maxsafefront(c->start);
            dist_base min = d_minsafefront(c->start) - L_TRAIN;
            distance &pointD = c->start;
            distance &pointE = c->end;
            if (pointC.max-max < 0 && pointE.min-min > 0) {
                track_condition_profile_external ext;
                if (pointD.max-max > 0)
                    ext.start = pointD.max-max;
                ext.end = pointE.min-min;
                c->external = ext;
            } else {
                c->external = {};
            }
        }
        ++it;
    }
}
void load_track_condition_traction(TrackConditionChangeTractionSystem cond, distance ref)
{
    for (auto it = track_conditions.begin(); it != track_conditions.end();) {
        if (it->get()->condition == TrackConditions::ChangeOfTractionSystem)
            it = track_conditions.erase(it);
        else
            ++it;
    }
    track_condition_traction_change *tc = new track_condition_traction_change();
    tc->start = ref + cond.D_TRACTION.get_value(cond.Q_SCALE);
    tc->condition = TrackConditions::ChangeOfTractionSystem;
    tc->profile = false;
    tc->m_voltage = cond.M_VOLTAGE;
    tc->nid_ctraction = cond.NID_CTRACTION;
    TractionSystem_DMI traction;
    if (cond.M_VOLTAGE.rawdata == cond.M_VOLTAGE.NonFitted) traction = TractionSystem_DMI::NonFitted;
    else if (cond.M_VOLTAGE.rawdata == cond.M_VOLTAGE.DC600V) traction = TractionSystem_DMI::DC750V;
    else if (cond.M_VOLTAGE.rawdata == cond.M_VOLTAGE.DC1k5V) traction = TractionSystem_DMI::DC1500V;
    else if (cond.M_VOLTAGE.rawdata == cond.M_VOLTAGE.DC3kV) traction = TractionSystem_DMI::DC3000V;
    else if (cond.M_VOLTAGE.rawdata == cond.M_VOLTAGE.AC15kV16Hz7) traction = TractionSystem_DMI::AC15kV;
    else if (cond.M_VOLTAGE.rawdata == cond.M_VOLTAGE.AC25kV50Hz) traction = TractionSystem_DMI::AC25kV;
    tc->start_symbol = PlanningTrackCondition(traction, true);
    tc->announcement_symbol = 25 + cond.M_VOLTAGE.rawdata*2 + (automatic_traction_system_change ? 0 : 1);
    tc->active_symbol = 23 + cond.M_VOLTAGE.rawdata*2 + (automatic_traction_system_change ? 0 : 1);
    tc->end_active_symbol = tc->active_symbol;
    track_conditions.insert(std::shared_ptr<track_condition_traction_change>(tc));
}
void load_track_condition_bigmetal(TrackConditionBigMetalMasses cond, distance ref)
{
    distance first = ref + cond.element.D_TRACKCOND.get_value(cond.Q_SCALE);
    for (auto it = track_conditions.begin(); it != track_conditions.end();) {
        if (it->get()->condition == TrackConditions::BigMetalMasses) {
            if (it->get()->start.max >= first.min) {
                it = track_conditions.erase(it);
                continue;
            }
            if (it->get()->end.min > first.max)
                it->get()->end = first;
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
        track_conditions.insert(std::shared_ptr<track_condition>(tc));
    }
}
void load_track_condition_various(TrackCondition cond, distance ref, bool special)
{
    if (cond.Q_TRACKINIT == cond.Q_TRACKINIT.InitialState) {
        distance resume = ref + cond.D_TRACKINIT.get_value(cond.Q_SCALE);
        for (auto it = track_conditions.begin(); it != track_conditions.end();) {
            TrackConditions c = it->get()->condition;
            if (((c == TrackConditions::SoundHorn || c == TrackConditions::NonStoppingArea || c == TrackConditions::TunnelStoppingArea) && special) ||
                ((c == TrackConditions::PowerLessSectionLowerPantograph || c == TrackConditions::PowerLessSectionSwitchMainPowerSwitch || 
                c == TrackConditions::RadioHole || c == TrackConditions::AirTightness || c == TrackConditions::SwitchOffRegenerativeBrake ||
                c == TrackConditions::SwitchOffEddyCurrentEmergencyBrake || c == TrackConditions::SwitchOffEddyCurrentServiceBrake || c == TrackConditions::SwitchOffMagneticShoe) && !special)) {
                
                if (it->get()->start.max >= resume.min) {
                    it = track_conditions.erase(it);
                    continue;
                }
                if (it->get()->end.min > resume.max)
                    it->get()->end = resume;
            }
            ++it;
        }
        return;
    }
    distance first = ref + cond.element.D_TRACKCOND.get_value(cond.Q_SCALE);
    for (auto it = track_conditions.begin(); it != track_conditions.end();) {
        TrackConditions c = it->get()->condition;
        if (((c == TrackConditions::SoundHorn || c == TrackConditions::NonStoppingArea || c == TrackConditions::TunnelStoppingArea) && special) ||
            ((c == TrackConditions::PowerLessSectionLowerPantograph || c == TrackConditions::PowerLessSectionSwitchMainPowerSwitch || 
            c == TrackConditions::RadioHole || c == TrackConditions::AirTightness || c == TrackConditions::SwitchOffRegenerativeBrake ||
            c == TrackConditions::SwitchOffEddyCurrentEmergencyBrake || c == TrackConditions::SwitchOffEddyCurrentServiceBrake || c == TrackConditions::SwitchOffMagneticShoe) && !special)) {
            
            if (it->get()->start.max >= first.min) {
                it = track_conditions.erase(it);
                continue;
            }
            if (it->get()->end.min > first.max)
                it->get()->end = first;
        }
        ++it;
    }
    std::vector<TC_element_packet> elements;
    elements.push_back(cond.element);
    elements.insert(elements.end(), cond.elements.begin(), cond.elements.end());
    distance curr = ref;
    for (auto it = elements.begin(); it != elements.end(); ++it) {
        bool s = it->M_TRACKCOND == it->M_TRACKCOND.NonStoppingArea || it->M_TRACKCOND == it->M_TRACKCOND.TunnelStoppingArea || it->M_TRACKCOND == it->M_TRACKCOND.SoundHorn;
        if (s != special) {
            curr += it->D_TRACKCOND.get_value(cond.Q_SCALE);
            continue;
        }
        if ((traction_systems.empty() || (traction_systems.size() == 1 && traction_systems.front().electrification == NonElectrical)) &&
        (it->M_TRACKCOND == it->M_TRACKCOND.PowerlessLowerPantograph || it->M_TRACKCOND == it->M_TRACKCOND.PowerlessSwitchOffPower || it->M_TRACKCOND == it->M_TRACKCOND.SwitchOffRegenerative)) {
            curr += it->D_TRACKCOND.get_value(cond.Q_SCALE);
            continue;
        }
        track_condition *tc = new track_condition();
        tc->start = curr + it->D_TRACKCOND.get_value(cond.Q_SCALE);
        curr = tc->start;
        tc->end = tc->start + it->L_TRACKCOND.get_value(cond.Q_SCALE);
        tc->profile = true;
        if (it->M_TRACKCOND == it->M_TRACKCOND.AirTightness) {
            tc->condition = TrackConditions::AirTightness;
            if (Q_airtight) {
                tc->start_symbol = PlanningTrackCondition(TrackConditionType_DMI::CloseAirIntake, !automatic_close_air_intake);
                tc->end_symbol = PlanningTrackCondition(TrackConditionType_DMI::OpenAirIntake, !automatic_open_air_intake);
                tc->announcement_symbol = automatic_open_air_intake ? 19 : 21;
                tc->active_symbol = 19;
                tc->end_active_symbol = automatic_close_air_intake ? 20 : 22;
            }
        } else if (it->M_TRACKCOND == it->M_TRACKCOND.TunnelStoppingArea) {
            tc->condition = TrackConditions::TunnelStoppingArea;
        } else if (it->M_TRACKCOND == it->M_TRACKCOND.NonStoppingArea) {
            tc->condition = TrackConditions::NonStoppingArea;
            tc->start_symbol = PlanningTrackCondition(TrackConditionType_DMI::NonStoppingArea, true);
            tc->active_symbol = 10;
            tc->announcement_symbol = 11;
        } else if (it->M_TRACKCOND == it->M_TRACKCOND.SoundHorn) {
            tc->condition = TrackConditions::SoundHorn;
            tc->start_symbol = PlanningTrackCondition(TrackConditionType_DMI::SoundHorn, true);
            tc->active_symbol = 35;
        } else if (it->M_TRACKCOND == it->M_TRACKCOND.PowerlessLowerPantograph) {
            tc->condition = TrackConditions::PowerLessSectionLowerPantograph;
            tc->start_symbol = PlanningTrackCondition(TrackConditionType_DMI::LowerPantograph, !automatic_lower_pantograph);
            tc->end_symbol = PlanningTrackCondition(TrackConditionType_DMI::RaisePantograph, !automatic_raise_pantograph);
            tc->announcement_symbol = automatic_lower_pantograph ? 2 : 3;
            tc->active_symbol = 1;
            tc->end_active_symbol = automatic_raise_pantograph ? 4 : 5;
        } else if (it->M_TRACKCOND == it->M_TRACKCOND.PowerlessSwitchOffPower) {
            tc->condition = TrackConditions::PowerLessSectionSwitchMainPowerSwitch;
            tc->start_symbol = PlanningTrackCondition(TrackConditionType_DMI::NeutralSectionAnnouncement, !automatic_open_power_switch);
            tc->end_symbol = PlanningTrackCondition(TrackConditionType_DMI::EndOfNeutralSection, !automatic_close_power_switch);
            tc->announcement_symbol = automatic_open_power_switch ? 6 : 7;
            tc->active_symbol = 6;
            tc->end_active_symbol = automatic_close_power_switch ? 8 : 9;
        } else if (it->M_TRACKCOND == it->M_TRACKCOND.RadioHole) {
            tc->condition = TrackConditions::RadioHole;
            tc->start_symbol = PlanningTrackCondition(TrackConditionType_DMI::RadioHole, false);
            tc->active_symbol = 12;
        } else if (it->M_TRACKCOND == it->M_TRACKCOND.SwitchOffRegenerative) {
            tc->condition = TrackConditions::SwitchOffRegenerativeBrake;
            tc->start_symbol = PlanningTrackCondition(TrackConditionType_DMI::RegenerativeBrakeInhibition, !automatic_regenerative_inhibition);
            tc->announcement_symbol = automatic_regenerative_inhibition ? 17 : 18;
            tc->active_symbol = 17;
        } else if (it->M_TRACKCOND == it->M_TRACKCOND.SwitchOffShoe) {
            tc->condition = TrackConditions::SwitchOffMagneticShoe;
            tc->start_symbol = PlanningTrackCondition(TrackConditionType_DMI::MagneticShoeInhibition, !automatic_magnetic_inhibition);
            tc->announcement_symbol = automatic_magnetic_inhibition ? 13 : 14;
            tc->active_symbol = 13;
        } else if (it->M_TRACKCOND == it->M_TRACKCOND.SwitchOffEddyService) {
            tc->condition = TrackConditions::SwitchOffEddyCurrentServiceBrake;
            tc->start_symbol = PlanningTrackCondition(TrackConditionType_DMI::EddyCurrentBrakeInhibition, !automatic_eddy_inhibition);
            tc->announcement_symbol = automatic_eddy_inhibition ? 15 : 16;
            tc->active_symbol = 15;
        } else if (it->M_TRACKCOND == it->M_TRACKCOND.SwitchOffEddyEmergency) {
            tc->condition = TrackConditions::SwitchOffEddyCurrentEmergencyBrake;
            tc->start_symbol = PlanningTrackCondition(TrackConditionType_DMI::EddyCurrentBrakeInhibition, !automatic_eddy_inhibition);
            tc->announcement_symbol = automatic_eddy_inhibition ? 15 : 16;
            tc->active_symbol = 15;
        }
        bool exists = false;
        for (auto it = track_conditions.begin(); it != track_conditions.end(); ++it) {
            track_condition *tc2 = it->get();
            if (tc2->condition == tc->condition) {
                if (tc->start.max <= tc2->end.min) {
                    exists = true;
                    if (tc2->end.min < tc->end.min) {
                        tc2->end = tc->end;
                        tc->targets.clear();
                    }
                }
            } 
        }
        if (exists)
            delete tc;
        else
            track_conditions.insert(std::shared_ptr<track_condition>(tc));
        
    }
    update_brake_contributions();
}
void load_track_condition_platforms(TrackConditionStationPlatforms cond, distance ref)
{
    if (cond.Q_TRACKINIT == cond.Q_TRACKINIT.InitialState) { 
        distance resume = ref + cond.D_TRACKINIT.get_value(cond.Q_SCALE);
        for (auto it = track_conditions.begin(); it != track_conditions.end(); ) {
            if (it->get()->condition == TrackConditions::StationPlatform) {
                if (it->get()->start.max >= resume.min) {
                    it = track_conditions.erase(it);
                    continue;
                }
                if (it->get()->end.min > resume.max)
                    it->get()->end = resume;
            }
            ++it;
        }
    }
    distance first = ref + cond.element.D_TRACKCOND.get_value(cond.Q_SCALE);
    for (auto it = track_conditions.begin(); it != track_conditions.end();) {
        if (it->get()->condition == TrackConditions::StationPlatform) {
            if (it->get()->start.max >= first.min) {
                it = track_conditions.erase(it);
                continue;
            }
            if (it->get()->end.min > first.max)
                it->get()->end = first;
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
        tc->right_side = it->Q_PLATFORM != it->Q_PLATFORM.LeftSide;
        tc->left_side = it->Q_PLATFORM != it->Q_PLATFORM.RightSide;
        tc->condition = TrackConditions::StationPlatform;
        track_conditions.insert(std::shared_ptr<track_condition>(tc));
    }
}