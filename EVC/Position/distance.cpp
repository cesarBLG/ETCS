/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "distance.h"
#include "../Supervision/supervision.h"
#include "../Supervision/national_values.h"
#include "linking.h"
#include <limits>
#define DISTANCE_COW
distance *distance::begin = nullptr;
distance *distance::end = nullptr;
dist_base dist_base::max = dist_base(std::numeric_limits<double>::max(), 0);
dist_base dist_base::min = dist_base(std::numeric_limits<double>::lowest(), 0);
distance::distance()
{
    if (begin == nullptr)
        begin = this;
    else
        prev = end;
    if (end != nullptr)
        end->next = this;
    end = this;
}
distance::distance(double val, int orientation, double ref) : min(val, orientation), est(val, orientation), max(val, orientation), ref(ref, orientation)
{
    if (begin == nullptr)
        begin = this;
    else
        prev = end;
    if (end != nullptr)
        end->next = this;
    end = this;
}
distance::distance(const distance &d) : min(d.min), est(d.est), max(d.max), ref(d.ref), balise_based(d.balise_based)
#if BASELINE == 4
, relocated_c(d.relocated_c)
, relocated_c_earlier(d.relocated_c_earlier)
#endif
{
    if (begin == nullptr)
        begin = this;
    else
        prev = end;
    if (end != nullptr)
        end->next = this;
    end = this;
}
distance::~distance()
{
    if (prev == nullptr)
        begin = next;
    else
        prev->next = next;
    if (next == nullptr)
        end = prev;
    else
        next->prev = prev;
}
distance &distance::operator = (const distance& d)
{
#if BASELINE == 4
    relocated_c = d.relocated_c;
    relocated_c_earlier = d.relocated_c_earlier;
#endif
    balise_based = d.balise_based;
    ref = d.ref;
    min = d.min;
    est = d.est;
    max = d.max;
    return *this;
}
distance distance::from_odometer(const dist_base &dist)
{
    distance d;
    d.balise_based = false;
    d.max = d.est = d.min = dist;
    d.ref = d_estfront;
    return d;
}
dist_base &dist_base::operator=(const dist_base &d)
{
    dist = d.dist;
    orientation = d.orientation;
    return *this;
}
dist_base &dist_base::operator+=(const double d)
{
    if (dist <= std::numeric_limits<double>::lowest() || dist >= std::numeric_limits<double>::max()) return *this;
    dist += orientation * d;
    return *this;
}
double dist_base::operator-(const dist_base &d) const
{
    int dir = 1;
    if (orientation * d.orientation < 0) abort();
    if (orientation == -1 || d.orientation == -1) dir = -1;
    if (dist <= std::numeric_limits<double>::lowest() ||
    d.dist >= std::numeric_limits<double>::max())
        return std::numeric_limits<double>::lowest();
    if (dist >= std::numeric_limits<double>::max() ||
    d.dist <= std::numeric_limits<double>::lowest())
        return std::numeric_limits<double>::max();
    return dir*(dist-d.dist);
}
bool dist_base::operator<(const dist_base &d) const
{
    if (orientation * d.orientation < 0) abort();
    int dir = 1;
    if (orientation == -1 || d.orientation == -1) dir = -1;
    if (dist <= std::numeric_limits<double>::lowest() ||
    d.dist <= std::numeric_limits<double>::lowest() ||
    dist >= std::numeric_limits<double>::max() ||
    d.dist >= std::numeric_limits<double>::max()) dir = 1;
    return dir == -1 ? dist>d.dist : dist<d.dist;
}
confidence_data confidence_data::from_distance(const distance &d)
{
    confidence_data c;
    c.ref = d.ref;
    c.locacc = 0;
    if (d.balise_based) {
#if BASELINE < 4
        if (d.ref.dist != 0) {
            c.locacc = Q_NVLOCACC;
            return c;
        }
#endif
        if (!solr)
            abort();
        c.locacc = solr->locacc;
    }
    return c;
}
confidence_data confidence_data::basic()
{
    confidence_data c;
    c.ref = dist_base(0,odometer_orientation);
    c.locacc = 0;
    if (solr)
        c.locacc = solr->locacc;
    return c;
}
dist_base d_maxsafe(const dist_base &d, const confidence_data &conf)
{
    if (d > conf.ref)
        return conf.ref + (d-conf.ref)*1.01 + conf.locacc;
    else
        return conf.ref + (d-conf.ref)*0.99 + conf.locacc;
}
dist_base d_minsafe(const dist_base &d, const confidence_data &conf)
{
    if (d > conf.ref)
        return conf.ref + (d-conf.ref)*0.99 - conf.locacc;
    else
        return conf.ref + (d-conf.ref)*1.01 - conf.locacc;
}
dist_base d_maxsafefront(const confidence_data &conf)
{
    return d_maxsafe(d_estfront, conf);
}
dist_base d_minsafefront(const confidence_data &conf)
{
    return d_minsafe(d_estfront, conf);
}
dist_base d_maxsafefront(const distance&ref)
{
    return d_maxsafe(d_estfront, confidence_data::from_distance(ref));
}
dist_base d_minsafefront(const distance&ref)
{
    return d_minsafe(d_estfront, confidence_data::from_distance(ref));
}
dist_base d_estfront(0,0);
dist_base d_estfront_dir[2] = {dist_base(0,1),dist_base(0,-1)};
double odometer_value=0;
double odometer_reference;
int odometer_orientation=1;
int current_odometer_orientation=1;
int odometer_direction=1;
void update_odometer()
{
    d_estfront = dist_base(odometer_value-odometer_reference,0);
    d_estfront_dir[0] = dist_base(odometer_value-odometer_reference,1);
    d_estfront_dir[1] = dist_base(odometer_value-odometer_reference,-1);
}
void reset_odometer(double dist)
{
    odometer_reference += dist;
    update_odometer();
}
