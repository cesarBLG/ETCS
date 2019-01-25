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

#include "variable.h"
#include <iostream>
using namespace std;
vector<data> variable::read(BitReader &bits)
{
    vector<data> variables;
    unsigned long val = 0;
    if(size!=0)
    {
        val = bits.read(size);
        variables.push_back({this, (int)val});
    }
    while(goInto(val))
    {
        for(int i=0; i<nested.size(); i++)
        {
            vector<data> nr = nested[i]->read(bits);
            variables.insert(variables.end(), nr.begin(), nr.end());
        }
    }
    return variables;
}
void variable::printall()
{
    cout<<name<<" "<<size<<endl;
    for(int i=0; i<nested.size(); i++)
    {
        nested[i]->printall();
    }
}
