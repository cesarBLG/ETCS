/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "targets.h"
#include "speed_profile.h"
#include "../Packets/52.h"
class PBD_target : public target
{
    public:
    distance start;
    distance end;
    double d_PBD;
    bool emergency;
    speed_restriction restriction;
    PBD_target(distance start, distance end, double d_PBD, bool emergency, double grad) : start(start), end(end), d_PBD(d_PBD), restriction(0, start, end, false), target(start.max+d_PBD, 0, target_class::PBD)
    {
        default_gradient = grad;
        is_EBD_based = emergency;
        use_brake_combination = false;
        calculate_decelerations();
    }
    void calculate_times() const override
    {
        if (conversion_model_used) {
            if (V_target > 0) {
                T_brake_emergency = T_brake_emergency_cmt;
                T_brake_service = T_brake_service_cmt;
            } else {
                T_brake_emergency = T_brake_emergency_cm0;
                T_brake_service = T_brake_service_cm0;
            }
        } else {
            T_brake_service = get_T_brake_service(d_estfront);
            T_brake_emergency = get_T_brake_emergency(d_estfront);
        }
        T_be = (conversion_model_used ? Kt_int : 1)*T_brake_emergency;
        T_bs = T_brake_service;
        T_bs1 = T_bs2 = T_bs;
        T_traction = T_traction_cutoff;
        T_berem = std::max(0.0, T_be-T_traction);
    }
    void calculate_decelerations() override
    {
        std::map<dist_base,double> gradient;
        target::calculate_decelerations(gradient);
        calculate_restriction();
    }
    void calculate_restriction();
};
extern std::list<std::shared_ptr<PBD_target>> PBDs;
void load_PBD(PermittedBrakingDistanceInformation &pbd, distance ref);