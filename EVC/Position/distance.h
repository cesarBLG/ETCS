#pragma once
#include <set>
#include <limits>
struct _dist_base
{
    double dist;
    double ref;
    _dist_base(double dist, double ref=0) : dist(dist), ref(ref) {}
    _dist_base(_dist_base &d) = default;
    double get()
    {
        return ref+dist;
    }
};
class distance
{
private:
    _dist_base *base;
    static std::set<_dist_base*> distances;
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
extern distance d_estfront;
extern double odometer_value;
void update_odometer();
void reset_odometer(double dist);
