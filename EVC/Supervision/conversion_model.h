#pragma once
#include <set>
#include <functional>
#include "distance.h"
extern const double T_bs1_locked;
extern const double T_bs2_locked;
extern double T_brake_service;
extern double T_bs;
extern double T_bs1;
extern double T_bs2;
struct acceleration
{
    std::set<distance> dist_step;
    std::set<double> speed_step;
    std::function<double(double, distance)> accel;
    double operator()(double V, distance d)
    {
        return accel(V,d);
    }
    friend acceleration operator+(const acceleration a1, const acceleration a2);
    
};
acceleration operator+(const acceleration a1, const acceleration a2);
extern acceleration A_brake_emergency;
extern acceleration A_brake_safe;
extern acceleration A_safe;
extern acceleration A_brake_service;
extern acceleration A_expected;
extern acceleration A_brake_normal_service;
extern acceleration A_normal_service;
extern acceleration A_gradient;

void set_test_values();
void calculate_gradient();
void calculate();
