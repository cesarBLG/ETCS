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
#pragma once
#include <set>
#include <map>
#include <vector>
#include "../optional.h"
#include "../Position/distance.h"
#include "../SSP/ssp.h"
#include "fixed_values.h"
#include "train_data.h"
void recalculate_MRSP();
void delete_track_info();
void delete_SSP();
void delete_gradient();
void delete_TSR();
std::map<distance,double> get_MRSP();
inline double dV_ebi(double vel)
{
    return std::max(dV_ebi_min, std::min(dV_ebi_min*(dV_ebi_max - dV_ebi_min)/(V_ebi_max-V_ebi_min)*(vel-V_ebi_min), dV_ebi_max));
}
inline double dV_sbi(double vel)
{
    return std::max(dV_sbi_min, std::min(dV_sbi_min*(dV_sbi_max - dV_sbi_min)/(V_sbi_max-V_sbi_min)*(vel-V_sbi_min), dV_sbi_max));
}
inline double dV_warning(double vel)
{
    return std::max(dV_warning_min, std::min(dV_warning_min*(dV_warning_max - dV_warning_min)/(V_warning_max-V_warning_min)*(vel-V_warning_min), dV_warning_max));
}
class speed_restriction
{
    double speed;
    distance start_distance;
    distance end_distance;
    bool compensate_train_length;
public:
    speed_restriction(double spd, distance start, distance end, bool compensate_trainlength) : speed(spd), start_distance(start), end_distance(end), compensate_train_length(compensate_trainlength) {}
    double get_speed() const { return speed; }
    distance get_start() const { return start_distance; }
    distance get_end() const { return end_distance + compensate_train_length*L_TRAIN; }
    distance get_uncompensated_end() const { return end_distance; }
    bool operator<(const speed_restriction r) const
    {
        if (start_distance == r.start_distance) {
            if (speed==r.speed)
                return get_end()<r.get_end();
            return speed<r.speed;
        }
        return start_distance<r.start_distance;
    }
    bool is_compensated() const
    {
        return compensate_train_length;
    }
};
void set_train_max_speed(double vel);
void update_SSP(std::vector<SSP_element> nSSP);
std::set<speed_restriction> get_SSP();
void update_gradient(std::map<distance, double> grad);
const std::map<distance, double> &get_gradient();
struct TSR
{
    int id;
    bool revocable;
    speed_restriction restriction;
};
void insert_TSR(TSR rest);
void revoke_TSR(int id_tsr);
extern optional<speed_restriction> SR_speed;
extern optional<speed_restriction> SH_speed;
extern optional<speed_restriction> UN_speed;
extern optional<speed_restriction> OS_speed;
extern optional<speed_restriction> LS_speed;
extern optional<speed_restriction> override_speed;
speed_restriction get_PBD_restriction(double d_PBD, distance start, distance end, bool EB, double g);
