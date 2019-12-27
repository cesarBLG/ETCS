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

#ifndef PARSED_PACKET_H
#define PARSED_PACKET_H
#include <deque>
#include <string>
#include "variable.h"
using namespace std;
class parsed_packet
{
    deque<data> variables;
public:
    string name;
    int num;
    parsed_packet(string name, deque<data> variables) : name(name), variables(variables)
    {

    }
    int peek(string varname, int &value)
    {
        if(variables[0].type->name != name) return -1;
        value = variables[0].val;
        return 0;
    }
    int read(string varname, int &value)
    {
        if(variables[0].type->name != name) return -1;
        value = variables[0].val;
        variables.pop_front();
        return 0;
    }
};

#endif // PARSED_PACKET_H
