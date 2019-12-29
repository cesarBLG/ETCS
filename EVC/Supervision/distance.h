#pragma once
#include <set>
#include <limits>
class distance
{
private:
    double *dist;
    //static std::set<double*> distances;
public:
    static void update_distances(double offset);
    double get() const
    {
        return *dist;
    }
    distance();
    distance(double val);
    distance(const distance &d);
    distance(distance &&d);
    ~distance();
    distance &operator = (const distance& d);
    distance &operator = (distance&& d);
    bool operator<(const distance d) const
    {
        return *dist<*d.dist;
    }
    bool operator>(const distance d) const
    {
        return *dist>*d.dist;
    }
    bool operator==(const distance d) const
    {
        return *dist==*d.dist;
    }
    bool operator!=(const distance d) const
    {
        return *dist!=*d.dist;
    }
    bool operator<=(const distance d) const
    {
        return *dist<=*d.dist;
    }
    bool operator>=(const distance d) const
    {
        return *dist>=*d.dist;
    }
    distance operator+(const double d) const
    {
        return distance(*dist+d);
    }
    distance operator-(const double d) const
    {
        return distance(*dist-d);
    }
    distance &operator+=(const double d)
    {
        (*dist) += d;
        return *this;
    }
    distance &operator-=(const double d)
    {
        (*dist) -= d;
        return *this;
    }
    double operator-(const distance d) const
    {
        return *dist-*d.dist;
    }
};
extern distance d_maxsafefront;
extern distance d_minsafefront;
extern distance d_estfront;
void update_train_position();
