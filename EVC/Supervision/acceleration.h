#pragma once
#include <set>
#include <functional>
#include "../Position/distance.h"
struct acceleration
{
    std::set<distance> dist_step;
    std::set<double> speed_step;
    std::function<double(double, distance)> accel;
    double operator()(double V, distance d) const
    {
        return accel(V,d);
    }
    friend acceleration operator+(const acceleration a1, const acceleration a2);
    
};
acceleration operator+(const acceleration a1, const acceleration a2);