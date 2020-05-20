#include "../TrackConditions/track_condition.h"
bool main_power_switch_status=true;
bool main_power_on_available=false;
bool main_power_off_available=true;
bool pantograph_status = true;
bool raise_pantograph_available=false;
bool lower_pantograph_available=true;
bool traction_cutoff_status;
bool traction_cutoff_available=true;
extern bool TCO;
void update_power_status()
{
    traction_cutoff_status = !TCO;
    bool power = true;
    bool panto = true;
    for (auto it = track_conditions.begin(); it != track_conditions.end(); ++it) {
        track_condition tc = *it->get();
        if (tc.condition == TrackConditions::PowerLessSectionSwitchMainPowerSwitch) {
            if (tc.get_distance_to_train() < V_est * 3 && tc.get_end_distance_to_train() > 0) {
                power = false;
            }
        }
        if (tc.condition == TrackConditions::PowerLessSectionLowerPantograph) {
            if (tc.get_distance_to_train() < V_est * 10 && tc.get_end_distance_to_train() > 0) {
                panto = false;
            }
        }
    }
    if (!power && main_power_switch_status)
        main_power_switch_status = false;
    if (power && !main_power_switch_status)
        main_power_switch_status = true;
    if (!panto && pantograph_status)
        pantograph_status = false;
    if (panto && !pantograph_status)
        pantograph_status = true;
}