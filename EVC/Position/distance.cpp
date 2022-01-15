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
#include "distance.h"
#include "../Supervision/supervision.h"
#include "../Supervision/national_values.h"
#include "linking.h"
#include <limits>
#include <iostream>
#define DISTANCE_COW
distance *distance::begin = nullptr;
distance *distance::end = nullptr;
void distance::update_distances(double expected, double estimated)
{
    distance *d = begin;
    while (d != nullptr) {
        if (d->ref == 0) {
            if (d->dist>std::numeric_limits<double>::lowest() && d->dist<std::numeric_limits<double>::max())
                d->dist -= expected;
        } else {
            d->ref -= estimated;
        }
        d = d->next;
    }
    update_odometer();
    V_release = calculate_V_release();
}
void distance::update_unlinked_reference(double newref)
{
    distance *d = begin;
    while (d != nullptr) {
        if (d->ref !=0 ) {
            double offset = newref-d->ref;
            d->ref = newref;
            if (d->dist>std::numeric_limits<double>::lowest() && d->dist<std::numeric_limits<double>::max())
                d->dist -= offset;
        }
        d = d->next;
    }
}
distance::distance() : dist(0), ref(0), orientation(0)
{
    if (begin == nullptr)
        begin = this;
    else
        prev = end;
    if (end != nullptr)
        end->next = this;
    end = this;
}
distance::distance(double val, int orientation, double ref) : dist(val), ref(ref), orientation(orientation)
{
    if (begin == nullptr)
        begin = this;
    else
        prev = end;
    if (end != nullptr)
        end->next = this;
    end = this;
}
distance::distance(const distance &d) : dist(d.dist), ref(d.ref), orientation(d.orientation)
{
    if (begin == nullptr)
        begin = this;
    else
        prev = end;
    if (end != nullptr)
        end->next = this;
    end = this;
}
distance::distance(distance &&d) : dist(d.dist),ref(d.ref),orientation(d.orientation)
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
    dist = d.dist;
    ref = d.ref;
    orientation = d.orientation;
    return *this;
}
distance &distance::operator = (distance&& d)
{
    dist = d.dist;
    ref = d.ref;
    orientation = d.orientation;
    return *this;
}
bool distance::operator<(const distance d) const
{
    if (orientation * d.orientation < 0) abort();
    int dir = 1;
    if (orientation == -1 || d.orientation == -1) dir = -1;
    if (dist <= std::numeric_limits<double>::lowest() ||
    d.dist <= std::numeric_limits<double>::lowest() ||
    dist >= std::numeric_limits<double>::max() ||
    d.dist >= std::numeric_limits<double>::max()) dir = 1;
    return dir == -1 ? get()>d.get() : get()<d.get();
}
distance &distance::operator+=(const double d)
{
    if (dist <= std::numeric_limits<double>::lowest() || dist >= std::numeric_limits<double>::max()) return *this;
    if (orientation == 0) { abort();}
    dist += orientation * d;
    return *this;
}
double distance::operator-(const distance d) const
{
    int dir = 1;
    if (orientation * d.orientation < 0) abort();
    if (orientation == -1 || d.orientation == -1) dir = -1;
    return dir*(dist+ref-d.dist-d.ref);
}
distance d_maxsafefront(int orientation, double reference)
{
    return distance((d_estfront.get()-reference)*1.01+odometer_orientation*(reference==0 && !lrbgs.empty() ? lrbgs.back().locacc : Q_NVLOCACC), orientation, reference);
}
distance d_minsafefront(int orientation, double reference)
{
    return distance((d_estfront.get()-reference)*0.99-odometer_orientation*(reference==0 && !lrbgs.empty() ? lrbgs.back().locacc : Q_NVLOCACC), orientation, reference);
}
distance d_maxsafefront(const distance&ref)
{
    return d_maxsafefront(ref.get_orientation(), ref.get_reference());
}
distance d_minsafefront(const distance&ref)
{
    return d_minsafefront(ref.get_orientation(), ref.get_reference());
}
distance d_maxsafe(distance &d)
{
    int orientation = d.get_orientation();
    double reference = d.get_reference();
    return distance((d.get()-reference)*1.01+orientation*(reference==0 && !lrbgs.empty() ? lrbgs.back().locacc : Q_NVLOCACC), orientation, reference);
}
distance d_minsafe(distance &d)
{
    int orientation = d.get_orientation();
    double reference = d.get_reference();
    return distance((d.get()-reference)*0.99-orientation*(reference==0 && !lrbgs.empty() ? lrbgs.back().locacc : Q_NVLOCACC), orientation, reference);
}
distance d_estfront(0,0,0);
distance d_estfront_dir[2] = {distance(0,1,0),distance(0,-1,0)};
double odometer_value=0;
double odometer_reference;
int odometer_orientation=1;
int current_odometer_orientation=1;
int odometer_direction=1;
void update_odometer()
{
    d_estfront = distance(odometer_value-odometer_reference,0,0);
    d_estfront_dir[0] = distance(odometer_value-odometer_reference,1,0);
    d_estfront_dir[1] = distance(odometer_value-odometer_reference,-1,0);
}
void reset_odometer(double dist)
{
    odometer_reference += dist;
    update_odometer();
}
