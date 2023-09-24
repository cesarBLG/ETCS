/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <limits>
#include <cstdlib>
using std::abort;
#define DISTANCE_COW
extern double odometer_value;
extern double odometer_reference;
extern int odometer_orientation;
extern int current_odometer_orientation;
extern int odometer_direction;
struct dist_base
{
    double dist;
    int orientation;
    dist_base() = default;
    dist_base(double dist, int orientation) : dist(dist), orientation(orientation) {}
    dist_base &operator = (const dist_base& d);
    static dist_base max;
    static dist_base min;
    bool operator<(const dist_base &d) const;
    bool operator>(const dist_base &d) const
    {
        return d<*this;
    }
    bool operator==(const dist_base &d) const
    {
        return dist == d.dist;
    }
    bool operator!=(const dist_base &d) const
    {
        return dist != d.dist;
    }
    bool operator<=(const dist_base &d) const
    {
        return !(*this>d);
    }
    bool operator>=(const dist_base &d) const
    {
        return !(*this<d);
    }
    dist_base operator+(const double d) const
    {
        dist_base dist=*this;
        dist+=d;
        return dist;
    }
    dist_base operator-(const double d) const
    {
        dist_base dist=*this;
        dist+=-d;
        return dist;
    }
    dist_base &operator+=(const double d);
    dist_base &operator-=(const double d)
    {
        *this += -d;
        return *this;
    }
    double operator-(const dist_base &d) const;
};
struct distance;
struct confidence_data
{
    dist_base ref;
    double locacc;
    static confidence_data from_distance(const distance &d);
    static confidence_data basic();
};
struct confidenced_distance : dist_base, confidence_data
{
    confidenced_distance() = default;
    confidenced_distance(dist_base d)
    {
        (dist_base&)*this = d;
        ref = dist_base(0, orientation);
        locacc = 0;
    }
    confidenced_distance(dist_base d, confidence_data c)
    {
        (dist_base&)*this = d;
        (confidence_data&)*this = c;
    }
};
struct distance
{
    static distance* begin;
    static distance* end;
    distance *prev=nullptr;
    distance *next=nullptr;
    dist_base min;
    dist_base est;
    dist_base max;
    dist_base ref;
    bool balise_based=true;
    distance();
    ~distance();
    distance(double val, int orientation, double ref=0);
    distance(const distance &d);
    distance &operator = (const distance& d);
    distance &operator+=(double dist)
    {
        min += dist;
        est += dist;
        max += dist;
        return *this;
    }
    distance &operator-=(double dist)
    {
        min -= dist;
        est -= dist;
        max -= dist;
        return *this;
    }
    distance operator+(double dist) const
    {
        distance d = *this;
        d += dist;
        return d;
    }
    distance operator-(double dist) const
    {
        distance d = *this;
        d -= dist;
        return d;
    }
#if BASELINE == 4
    bool relocated_c;
    optional<bg_id> relocated_c_earlier;
#endif
    static distance from_odometer(const dist_base &dist);
};
extern dist_base d_estfront;
extern dist_base d_estfront_dir[2];
dist_base d_maxsafe(const dist_base &d, const confidence_data &conf);
dist_base d_minsafe(const dist_base &d, const confidence_data &conf);
dist_base d_maxsafefront(const confidence_data &conf);
dist_base d_minsafefront(const confidence_data &conf);
dist_base d_maxsafefront(const distance&ref);
dist_base d_minsafefront(const distance&ref);
void update_odometer();
void reset_odometer(double dist);
