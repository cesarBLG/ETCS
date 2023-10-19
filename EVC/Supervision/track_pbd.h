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
    PBD_target(distance start, distance end, double d_PBD, bool emergency, double grad) : start(start), end(end), d_PBD(d_PBD), restriction(0, start, end, false), target(/*start+d_PBD*/relocable_dist_base(), 0, target_class::PBD)
    {
        default_gradient = grad;
        is_EBD_based = emergency;
        use_brake_combination = false;
        calculate_decelerations();
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