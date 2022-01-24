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
#pragma once
#include "../Position/distance.h"
#include "../Supervision/train_data.h"
#include "../Packets/39.h"
#include "../Packets/40.h"
#include "../Packets/67.h"
#include "../Packets/68.h"
#include "../Packets/69.h"
#include "../optional.h"
#include <functional>
#include <list>
#include <memory>
enum struct TrackConditions
{
    PowerLessSectionLowerPantograph,
    PowerLessSectionSwitchMainPowerSwitch,
    AirTightness,
    SoundHorn,
    NonStoppingArea,
    TunnelStoppingArea,
    ChangeOfTractionSystem,
    ChangeOfAllowedCurrentConsumption,
    BigMetalMasses,
    RadioHole,
    SwitchOffRegenerativeBrake,
    SwitchOffEddyCurrentServiceBrake,
    SwitchOffEddyCurrentEmergencyBrake,
    SwitchOffMagneticShoe,
    StationPlatform
};
enum struct TrackConditionType_DMI
{
    None,
    LowerPantograph,
    RaisePantograph,
    NeutralSectionAnnouncement,
    EndOfNeutralSection,
    NonStoppingArea,
    RadioHole,
    MagneticShoeInhibition,
    EddyCurrentBrakeInhibition,
    RegenerativeBrakeInhibition,
    OpenAirIntake,
    CloseAirIntake,
    SoundHorn,
    TractionSystemChange,
    // Legacy conditions used in old DMIs
    Tunnel,
    Bridge,
    Station,
    EndOfTrack
};
enum struct TractionSystem_DMI
{
    NonFitted,
    AC25kV,
    AC15kV,
    DC3000V,
    DC1500V,
    DC750V
};
struct PlanningTrackCondition
{
    TrackConditionType_DMI Type;
    float DistanceToTrainM;
    bool YellowColour;
    TractionSystem_DMI TractionSystem;
    PlanningTrackCondition(TrackConditionType_DMI type, bool isYellowColour)
    {
        Type = type;
        YellowColour = isYellowColour;
    }
    PlanningTrackCondition(TractionSystem_DMI tractionSystem, bool isYellowColour)
    {
        Type = TrackConditionType_DMI::TractionSystemChange;
        YellowColour = isYellowColour;
        TractionSystem = tractionSystem;
    }
};
struct track_condition
{
    TrackConditions condition;
    distance start;
    bool profile;
    distance end;
    PlanningTrackCondition start_symbol;
    PlanningTrackCondition end_symbol;
    int active_symbol;
    int announcement_symbol;
    int end_active_symbol;
    double announce_distance;
    bool announce;
    bool order;
    bool display_end;
    bool end_displayed;
    int64_t end_time;
    track_condition() : start_symbol(TrackConditionType_DMI::None, false), end_symbol(TrackConditionType_DMI::None, false)
    {
        active_symbol = announcement_symbol = end_active_symbol = -1;
        announce = order = display_end = end_displayed = false;
    }
    virtual double get_distance_to_train()
    {
        if (condition == TrackConditions::BigMetalMasses)
            return start-d_maxsafefront(start)+L_antenna_front;
        if (condition == TrackConditions::TunnelStoppingArea || condition == TrackConditions::SoundHorn)
            return start-d_estfront;
        return start-d_maxsafefront(start);
    }
    double get_end_distance_to_train()
    {
        if (!profile) {
            if (condition == TrackConditions::ChangeOfTractionSystem)
                return start-d_minsafefront(start)+L_TRAIN;
            return get_distance_to_train();
        }
        if (condition == TrackConditions::BigMetalMasses)
            return end-d_minsafefront(end)+L_antenna_front;
        if (condition == TrackConditions::TunnelStoppingArea || condition == TrackConditions::SoundHorn)
            return end-d_estfront;
        if (condition == TrackConditions::PowerLessSectionLowerPantograph || condition == TrackConditions::PowerLessSectionSwitchMainPowerSwitch || condition == TrackConditions::StationPlatform)
            return end-d_minsafefront(end);
        return end-d_minsafefront(end)+L_TRAIN;
    }
};
struct track_condition_platforms : track_condition
{
    double platform_height;
    bool left_side;
    bool right_side;
};
extern std::list<std::shared_ptr<track_condition>> track_conditions;
extern optional<distance> restore_initial_states_various;
void update_track_conditions();
void update_brake_contributions();
void load_track_condition_bigmetal(TrackConditionBigMetalMasses cond, distance ref);
void load_track_condition_various(TrackCondition cond, distance ref, bool special);
void load_track_condition_platforms(TrackConditionStationPlatforms cond, distance ref);
void load_track_condition_traction(TrackConditionChangeTractionSystem cond, distance ref);