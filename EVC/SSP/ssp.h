#pragma once
#include "../Position/distance.h"
#include "../Packets/27.h"
#include <map>
#include <set>
#include <vector>
struct SSP_element
{
    distance start;
    bool compensate_train_length;
    std::map<int,double> restrictions[3];
    SSP_element() = default;
    SSP_element(distance start, double basic_speed, bool comp) : start(start), compensate_train_length(comp)
    {
        restrictions[0][-1] = basic_speed;
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
std::vector<SSP_element> get_SSP(distance start, InternationalSSP issp);