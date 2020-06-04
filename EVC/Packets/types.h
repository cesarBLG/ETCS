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
#include <stdint.h>
#include <vector>
#include "variables.h"
#include <typeinfo>
#include <iostream>
#include <string>
struct bit_read_temp
{
    std::vector<bool> bits;
    int position;
    bool error=false;
    bool sparefound=false;
    void read(ETCS_variable *var)
    {
        uint32_t value=0;
        int count=var->size;
        while(count-->0) {
            if (bits.size() <= position) {
                error = true;
                return;
            }
            value = value<<1 | bits[position++];
        }
        var->rawdata = value;
        if (!var->is_valid())
            sparefound = true;
        /*std::string tip = typeid(*var).name();
        tip = tip.substr(tip.find_first_not_of("0123456789"));
        std::cout<<tip.substr(0,tip.size()-2)<<"\t"<<var->rawdata<<std::endl;*/
    }
    void peek(ETCS_variable *var, int offset=0)
    {
        int position = this->position+offset;
        uint32_t value=0;
        int count=var->size;
        while(count-->0) {
            if (bits.size() <= position) {
                return;
            }
            value = value<<1 | bits[position++];
        }
        var->rawdata = value;
    }
    bit_read_temp(std::vector<bool> bits) : bits(bits), position(0) {}
};