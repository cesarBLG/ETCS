#pragma once
#include <set>
#include <map>
#include <vector>
#include "distance.h"
#include "fixed_values.h"
#include "train_data.h"
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
    speed_restriction(double spd, distance start, distance end, bool compensate_trainlength) : speed(spd), start_distance(start), end_distance(end), compensate_train_length(true) {}
    double get_speed() { return speed; }
    distance get_start() { return start_distance; }
    distance get_end() { return end_distance + compensate_train_length*L_TRAIN; }
};
struct SSP_element
{
    distance start;
    bool compensate_train_length;
    std::map<int,double> restrictions[3];
    SSP_element() = default;
    SSP_element(distance start, double basic_speed, bool comp) : start(start), compensate_train_length(comp)
    {
        restrictions[0][0] = basic_speed;
    }
    double get_speed(int train_cant_deficiency, std::set<int> train_categories)
    {
        double v = (--restrictions[0].upper_bound(train_cant_deficiency))->second;
        double v2 = 10000;
        bool replaces = false;
        for (int cat : train_categories) {
            auto it = restrictions[1].find(cat);
            if (it!=restrictions[1].end()) {
                v2 = std::min(v2, it->second);
                replaces = true;
            }
        }
        if (replaces)
            v = v2;
        for (int cat : train_categories) {
            auto it = restrictions[2].find(cat);
            if (it!=restrictions[2].end()) {
                v = std::min(v, it->second);
            }
        }
        return v;
    }
};
void update_SSP(std::vector<SSP_element> nSSP);
struct TSR
{
    int id;
    bool revocable;
    speed_restriction restriction;
};
void insert_TSR(TSR rest);
void revoke_TSR(int id_tsr);
class speed_restriction_list
{
private:
    std::map<distance,double> MRSP;
    std::set<speed_restriction*> restrictions;
    void recalculate_MRSP();
public:
    std::map<distance,double> get_MRSP()
    {
        return MRSP;
    }
    void insert_restriction(speed_restriction *r)
    {
        restrictions.insert(r);
        recalculate_MRSP();
    }
    void remove_restriction(speed_restriction *r)
    {
        restrictions.erase(r);
        recalculate_MRSP();
    }
};
extern speed_restriction_list mrsp_candidates;
