#pragma once
#include <set>
class distance
{
private:
    double *dist;
    static std::set<double*> distances;
    static void update_distances(double offset)
    {
        for(double *d : distances) {
            *d += offset;
        }
    }
public:
    double get() const
    {
        return *dist;
    }
    distance()
    {
        dist = new double(0);
        distances.insert(dist);
    }
    distance(double val)
    {
        dist = new double(val);
        distances.insert(dist);
    }
    distance(const distance &d)
    {
        dist = new double(*d.dist);
        distances.insert(dist);
    }
    distance(distance &&d)
    {
        dist = new double(*d.dist);
        distances.insert(dist);
    }
    ~distance()
    {
        distances.erase(dist);
        delete dist;
    }
    distance &operator = (const distance& d)
    {
        distances.erase(dist);
        delete dist;
        dist = new double(*d.dist);
        distances.insert(dist);
        return *this;
    }
    distance &operator = (distance&& d)
    {
        distances.erase(dist);
        delete dist;
        dist = new double(*d.dist);
        distances.insert(dist);
        return *this;
    }
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
