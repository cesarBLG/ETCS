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
#include <typeinfo>
#include <iostream>
#include <string>
template<typename T>
class ETCS_variable_custom;
struct bit_manipulator
{
    std::vector<bool> bits;
    std::vector<std::pair<std::string,std::string>> log_entries;
    bool write_mode;
    int position;
    bool error=false;
    bool sparefound=false;
    bit_manipulator()
    {
        write_mode = true;
    }
    bit_manipulator(std::vector<bool> &&bits) : bits(bits), position(0)
    {
        write_mode = false;
    }
    bit_manipulator(unsigned char *data, int count) : position(0) {
        write_mode = false;
        for (int i=0; i<count; i++) {
            for (int j=7; j>=0; j--) {
                bits.push_back(((data[i]>>j) & 1));
            }
        }
    }
    template<typename T>
    void log(ETCS_variable_custom<T> *var)
    {
        std::string name = typeid(*var).name();
        name = name.substr(name.find_first_not_of("0123456789"));
        name = name.substr(0, name.size()-2);
        log_entries.push_back({name, std::to_string(var->rawdata)});
    }
    template<typename T>
    void read(ETCS_variable_custom<T> *var)
    {
        T value=0;
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
        log(var);
    }
    template<typename T>
    void peek(ETCS_variable_custom<T> *var, int offset=0)
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
    template<typename T>
    void write(ETCS_variable_custom<T> *var)
    {
        T value=var->rawdata;
        int count=var->size;
        while(count-->0) {
            bits.push_back((value>>count)&1);
        }
        log(var);
    }
    template<typename T>
    void replace(ETCS_variable_custom<T> *var, int pos)
    {
        if (var->size + pos > bits.size()) return;
        for (int i=0; i<var->size; i++) {
            bits[pos+i] = (var->rawdata>>(var->size-i-1)) & 1;
        }
    }
    void get_bytes(unsigned char *data)
    {
        int div = bits.size()/8;
        int rem = bits.size()%8;
        for (int i=0; i<div; i++) {
            char c = 0;
            for (int j=0; j<8; j++) {
                c |= bits[8*i+j]<<(7-j);
            }
            data[i] = c;
        }
        if (rem > 0) {
            char c = 0;
            for (int i=0; i<rem; i++) {
                c |= bits[8*div+i]<<(7-i);
            }
            data[div] = c;
        }
    }
};
