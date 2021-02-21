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
#include <unordered_set>
#include <limits>
#define DISTANCE_COW
struct _dist_base
{
    double dist;
    double ref;
    int refcount;
    _dist_base(double dist, double ref=0) : dist(dist), ref(ref), refcount(1) {}
    _dist_base(const _dist_base &d) : dist(d.dist), ref(d.ref), refcount(1) {}
    double get()
    {
        return ref+dist;
    }
};
class distance
{
private:
    _dist_base *base;
#ifndef DISTANCE_COW
    _dist_base base_allocation;
#endif
    static std::unordered_set<_dist_base*> distances;
public:
    static void update_distances(double expected, double estimated);
    static void update_unlinked_reference(double newref);
    double get() const
    {
        return base->get();
    }
    double get_reference() const
    {
        return base->ref;
    }
    distance();
    distance(double val, double ref=0);
    distance(const distance &d);
    distance(distance &&d);
    ~distance();
    distance &operator = (const distance& d);
    distance &operator = (distance&& d);
    bool operator<(const distance d) const
    {
        return base->get()<d.base->get();
    }
    bool operator>(const distance d) const
    {
        return base->get()>d.base->get();
    }
    bool operator==(const distance d) const
    {
        return base->get()==d.base->get();
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
    distance &operator+=(const double d)
    {
#ifdef DISTANCE_COW
        if (base->refcount > 1)
        {
            base->refcount--;
            base = new _dist_base(*base);
            distances.insert(base);
        }
#endif
        base->dist += d;
        return *this;
    }
    distance &operator-=(const double d)
    {
        *this += -d;
        return *this;
    }
    double operator-(const distance d) const
    {
        return base->get()-d.base->get();
    }
};
distance d_maxsafefront(double reference);
distance d_minsafefront(double reference);
distance d_maxsafe(distance d, double reference);
distance d_minsafe(distance d, double reference);
extern distance d_estfront;
extern double odometer_value;
void update_odometer();
void reset_odometer(double dist);
