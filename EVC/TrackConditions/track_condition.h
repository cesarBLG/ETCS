/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
            return start.max-d_maxsafefront(start)+L_antenna_front;
        if (condition == TrackConditions::TunnelStoppingArea || condition == TrackConditions::SoundHorn)
            return start.est-d_estfront;
        return start.max-d_maxsafefront(start);
    }
    double get_end_distance_to_train()
    {
        if (!profile) {
            if (condition == TrackConditions::ChangeOfTractionSystem)
                return start.min-d_minsafefront(start)+L_TRAIN;
            return get_distance_to_train();
        }
        if (condition == TrackConditions::BigMetalMasses)
            return end.min-d_minsafefront(end)+L_antenna_front;
        if (condition == TrackConditions::TunnelStoppingArea || condition == TrackConditions::SoundHorn)
            return end.est-d_estfront;
        if (condition == TrackConditions::PowerLessSectionLowerPantograph || condition == TrackConditions::PowerLessSectionSwitchMainPowerSwitch || condition == TrackConditions::StationPlatform)
            return end.min-d_minsafefront(end);
        return end.min-d_minsafefront(end)+L_TRAIN;
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