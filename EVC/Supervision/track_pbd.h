/*
 * European Train Control System
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
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
    PBD_target(distance start, distance end, double d_PBD, bool emergency, double grad) : start(start), end(end), d_PBD(d_PBD), restriction(0, start, end, false), target(start+d_PBD, 0, target_class::PBD)
    {
        default_gradient = grad;
        is_EBD_based = emergency;
        use_brake_combination = false;
        calculate_decelerations();
    }
    void calculate_decelerations() override
    {
        std::map<distance,double> gradient;
        target::calculate_decelerations(gradient);
        calculate_restriction();
    }
    void calculate_restriction();
};
extern std::list<PBD_target> PBDs;
void load_PBD(PermittedBrakingDistanceInformation &pbd, distance ref);