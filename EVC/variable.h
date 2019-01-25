/*
 * European Train Control System
 * Copyright (C) 2019  César Benito <cesarbema2009@hotmail.com>
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

#ifndef _VARIABLE_H
#define _VARIABLE_H

/**
 * @todo write docs
 */
#include <string>
#include <vector>
#include "bitreader.h"
using namespace std;
class variable;
struct data
{
    variable *type;
    int val;
};
class variable
{
public:
    string name;
    const int size;
    vector<variable*> nested;
    virtual bool goInto(int val) {return false;}
public:
    variable() : size(0) {}
    void printall();
    variable(string name, int size) : name(name), size(size) {}
    virtual ~variable()
    {
        for(int i=0; i<nested.size(); i++)
        {
            delete nested[i];
        }
    }
    vector<data> read(BitReader &bits);
};
class condition : public variable
{
    int target = 0;
    bool entered = false;
    bool goInto(int val) override
    {
        if(entered) return false;
        entered = true;
        return val == target;
    }
public:
    condition(string name, int size, int target) : variable(name, size), target(target){}
};
class N_ITER : public variable
{
    int count = 0;
    bool goInto(int val) override
    {
        return count++<val;
    }
public:
    N_ITER() : variable("N_ITER", 5){}
};
#endif // _VARIABLE_H