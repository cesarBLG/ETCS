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
struct relocable_dist_base;
struct confidence_data
{
    dist_base ref;
    double locacc;
    static confidence_data from_distance(const relocable_dist_base &d);
    static confidence_data basic();
};
struct relocable_dist_base : dist_base
{
    static relocable_dist_base* begin;
    static relocable_dist_base* end;
    relocable_dist_base *prev=nullptr;
    relocable_dist_base *next=nullptr;
    dist_base ref;
    int type;
    bool balise_based=true;
#if BASELINE == 4
    bool relocated_c;
    optional<bg_id> relocated_c_earlier;
#endif
    relocable_dist_base();
    ~relocable_dist_base();
    relocable_dist_base(dist_base d, dist_base ref, int type=0);
    relocable_dist_base(const relocable_dist_base &d);
    relocable_dist_base& operator=(const relocable_dist_base &d);
    relocable_dist_base operator+(const double d) const
    {
        relocable_dist_base dist=*this;
        dist+=d;
        return dist;
    }
    relocable_dist_base operator-(const double d) const
    {
        relocable_dist_base dist=*this;
        dist+=-d;
        return dist;
    }
    double operator-(const dist_base &d) const
    {
        return dist_base::operator-(d);
    }
};
struct distance
{
    relocable_dist_base min;
    relocable_dist_base est;
    relocable_dist_base max;
    distance() = default;
    distance(double val, int orientation, double ref=0);
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
dist_base d_maxsafefront(const relocable_dist_base&ref);
dist_base d_minsafefront(const relocable_dist_base&ref);
void update_odometer();
void reset_odometer(double dist);
