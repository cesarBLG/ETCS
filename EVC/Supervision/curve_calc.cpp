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
#include <chrono>
#include <iostream>
#include <cmath>
#include "conversion_model.h"
distance distance_curve(const acceleration &a, const distance &dref, double vref, double vel)
{
    if (a.speed_step.empty() || vref<*a.speed_step.begin() || a.dist_step.empty() || dref<*a.dist_step.begin())
        return distance(std::numeric_limits<float>::min(), 0, 0);
    auto v = --a.speed_step.upper_bound(vref);
    auto d = --a.dist_step.upper_bound(dref);
    bool dec = 1; //Decceleration curve
    bool inc = vel>vref;
    bool fwd = dec != inc;
    auto vnext = inc ? next(v) : v;
    auto dnext = fwd ? next(d) : d;
    distance pos = dref;
    double v02 = vref*vref;
    double v2 = vel*vel;
    for (;;) {
        double dac = (dec ? -2 : 2)*a(*v,*d);
        bool vend = vnext == a.speed_step.end();
        bool dend = dnext == a.dist_step.end();
        double vv2 = vend ? (inc ? 1e9 : -1) : (*vnext)*(*vnext);
        double vd2 = (dend || dnext->get() <= std::numeric_limits<double>::lowest() || dnext->get() >= std::numeric_limits<double>::max()) ? (inc ? 1e9 : -1) : dac*(*dnext-pos)+v02;
        if (inc ? (v2<=std::min(vv2,vd2)) : (v2>=std::max(vv2,vd2))) {
            pos += (v2-v02)/dac;
            v02 = v2;
            break;
        } else if (inc ? (vv2<vd2) : (vv2>vd2)) {
            pos += (vv2-v02)/dac;
            v02 = vv2;
            if (inc) {
                v++;
                vnext++;
            } else {
                v--;
                vnext--;
            }
        } else {
            v02 = vd2;
            pos = *dnext;
            if (fwd) {
                d++;
                dnext++;
            } else {
                d--;
                dnext--;
            }
        }
    }
    return pos;
};
double speed_curve(const acceleration &a, const distance &dref, double vref, distance dist)
{
    if (a.speed_step.empty() || vref<*a.speed_step.begin() || a.dist_step.empty() || dref<*a.dist_step.begin())
        return 0;
    if (dist<*a.dist_step.begin())
        dist = *a.dist_step.begin();
    auto v = --a.speed_step.upper_bound(vref);
    auto d = --a.dist_step.upper_bound(dref);
    bool dec = 1; //Decceleration curve
    bool fwd = dist>dref;
    bool inc = dec != fwd;
    auto vnext = inc ? next(v) : v;
    auto dnext = fwd ? next(d) : d;
    distance pos = dref;
    double v02 = vref*vref;
    for (;;) {
        double dac = (dec ? -2 : 2)*a(*v,*d);
        bool vend = vnext == a.speed_step.end();
        bool dend = dnext == a.dist_step.end();
        double vv2 = vend ? (inc ? 1e9 : -1) : (*vnext)*(*vnext);
        double vd2 = (dend || dnext->get() <= std::numeric_limits<double>::lowest() || dnext->get() >= std::numeric_limits<double>::max()) ? (inc ? 1e9 : -1) : dac*(*dnext-pos)+v02;
        double v2 = std::max(dac*(dist-pos)+v02, 0.0);
        if (inc ? (v2<=std::min(vv2,vd2)) : (v2>=std::max(vv2,vd2))) {
            pos = dist;
            v02 = v2;
            break;
        } else if (inc ? (vv2<vd2) : (vv2>vd2)) {
            pos += (vv2-v02)/dac;
            v02 = vv2;
            if (inc) {
                v++;
                vnext++;
            } else {
                v--;
                vnext--;
            }
        } else {
            v02 = vd2;
            pos = *dnext;
            if (fwd) {
                d++;
                dnext++;
            } else {
                d--;
                dnext--;
            }
        }
    }
    return sqrt(v02);
}
