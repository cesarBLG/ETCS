#pragma once
#include "../Position/distance.h"
#include "../Supervision/train_data.h"
#include "../Packets/68.h"
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
struct track_condition
{
    TrackConditions condition;
    distance start;
    bool profile;
    distance end;
    int start_symbol;
    int end_symbol;
    int active_symbol;
    int announcement_symbol;
    int end_active_symbol;
    bool announce;
    bool order;
    bool display_end;
    int64_t end_time;
    track_condition() 
    {
        start_symbol = end_symbol = active_symbol = announcement_symbol = end_active_symbol = -1;
        announce = order = display_end = false;
    }
    virtual double get_distance_to_train()
    {
        if (condition == TrackConditions::BigMetalMasses)
            return start-d_maxsafefront(start.get_reference())+L_antenna_front;
        if (condition == TrackConditions::TunnelStoppingArea || condition == TrackConditions::SoundHorn)
            return start-d_estfront;
        return start-d_maxsafefront(start.get_reference());
    }
    double get_end_distance_to_train()
    {
        if (!profile)
            return get_distance_to_train();
        if (condition == TrackConditions::BigMetalMasses)
            return end-d_minsafefront(end.get_reference())+L_antenna_front;
        if (condition == TrackConditions::TunnelStoppingArea || condition == TrackConditions::SoundHorn)
            return end-d_estfront;
        if (condition == TrackConditions::PowerLessSectionLowerPantograph || condition == TrackConditions::PowerLessSectionSwitchMainPowerSwitch || condition == TrackConditions::StationPlatform)
            return end-d_minsafefront(end.get_reference());
        return end-d_minsafefront(end.get_reference())+L_TRAIN;
    }
};
extern std::list<std::shared_ptr<track_condition>> track_conditions;
extern optional<distance> restore_initial_states_various;
void update_track_conditions();
void load_track_condition_various(TrackCondition cond, distance ref);