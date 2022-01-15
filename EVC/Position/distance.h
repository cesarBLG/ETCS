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
#include <limits>
#include <cstdlib>
using std::abort;
#define DISTANCE_COW
extern double odometer_value;
extern double odometer_reference;
extern int odometer_orientation;
extern int current_odometer_orientation;
extern int odometer_direction;
/*struct _dist_base
{
    int refcount;
    _dist_base(double dist, int orientation, double ref=0) : dist(dist), ref(ref), refcount(1), orientation(orientation) {}
    _dist_base(const _dist_base &d) : dist(d.dist), ref(d.ref), refcount(1), orientation(d.orientation) {}
    double get()
    {
        return ref+dist;
    }
};*/
class distance
{
private:
    double dist;
    double ref;
    int orientation;
    static distance* begin;
    static distance* end;
    distance *prev=nullptr;
    distance *next=nullptr;
public:
    static void update_distances(double expected, double estimated);
    static void update_unlinked_reference(double newref);
    double get() const
    {
        return dist+ref;
    }
    double get_reference() const
    {
        return ref;
    }
    int get_orientation() const
    {
        return orientation;
    }
    distance();
    distance(double val, int orientation, double ref);
    distance(const distance &d);
    distance(distance &&d);
    ~distance();
    distance &operator = (const distance& d);
    distance &operator = (distance&& d);
    bool operator<(const distance d) const;
    bool operator>(const distance d) const
    {
        return d<*this;
    }
    bool operator==(const distance d) const
    {
        return get()==d.get();
    }
    bool operator!=(const distance d) const
    {
        return !(*this==d);
    }
    bool operator<=(const distance d) const
    {
        return !(*this>d);
    }
    bool operator>=(const distance d) const
    {
        return !(*this<d);
    }
    distance operator+(const double d) const
    {
        distance dist=*this;
        dist+=d;
        return dist;
    }
    distance operator-(const double d) const
    {
        distance dist=*this;
        dist+=-d;
        return dist;
    }
    distance &operator+=(const double d);
    distance &operator-=(const double d)
    {
        *this += -d;
        return *this;
    }
    double operator-(const distance d) const;
};
extern distance d_estfront;
extern distance d_estfront_dir[2];
distance d_maxsafefront(int orientation, double reference);
distance d_minsafefront(int orientation, double reference);
distance d_maxsafefront(const distance&ref);
distance d_minsafefront(const distance&ref);
distance d_maxsafe(distance &d);
distance d_minsafe(distance &d);
void update_odometer();
void reset_odometer(double dist);
