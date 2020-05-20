#include "override.h"
#include "mode_transition.h"
#include "../Supervision/national_values.h"
#include "../Supervision/speed_profile.h"
#include "../Time/clock.h"
bool overrideProcedure = false;
distance override_end_distance;
int64_t override_end_time;
optional<distance> formerEoA;
bool formerValid = false;
void start_override()
{
    if (V_est <= V_NVALLOWOVTRP) {    
        overrideProcedure = true;
        formerValid = false;
        override_end_distance = d_estfront + D_NVOVTRP;
        override_speed = speed_restriction(V_NVSUPOVTRP, distance(std::numeric_limits<double>::lowest()), override_end_distance, false);
        override_end_time = get_milliseconds() + T_NVOVTRP*1000;
        if (mode == Mode::OS || mode == Mode::LS || mode == Mode::FS) {
            if (EoA)
                formerEoA = EoA;
            else if (LoA)
                formerEoA = LoA->first;
            formerValid = formerEoA && *EoA > d_estfront;
        } else if (mode == Mode::PT || mode == Mode::SB) {
            formerEoA = d_estfront;
        }
        recalculate_MRSP();
        trigger_condition(37);
    }
}
void update_override()
{
    if (overrideProcedure) {
        if (d_estfront > override_end_distance || get_milliseconds() > override_end_time || MA)
            overrideProcedure = false;
        if (formerValid && formerEoA && *formerEoA<d_minsafefront(formerEoA->get_reference())-L_antenna_front)
            overrideProcedure = false;
        if (!overrideProcedure) {
            override_speed = {};
            recalculate_MRSP();
        }
    }
}