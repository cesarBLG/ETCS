/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <stdint.h>
#include <vector>
#include <typeinfo>
#include <string>
template<typename T>
class ETCS_variable_custom;
struct bit_manipulator
{
    std::vector<unsigned char> bits;
    std::vector<std::pair<std::string,std::string>> log_entries;
    bool write_mode;
    int position;
    bool error=false;
    bool sparefound=false;
    int m_version = 33;
    bit_manipulator() : position(0)
    {
        write_mode = true;
    }
    bit_manipulator(std::vector<unsigned char> &&bits) : bits(bits), position(0)
    {
        write_mode = false;
    }
    bit_manipulator(unsigned char *data, int count) : position(0)
    {
        write_mode = false;
        bits.insert(bits.end(), data, data+count);
    }
    bit_manipulator(std::string base64);
    inline bool operator[](int pos)
    {
        return bits[pos>>3] & (1<<(7-(pos&7)));
    }
    template<typename T>
    void log(ETCS_variable_custom<T> *var)
    {
        std::string name = typeid(*var).name();
#ifdef _MSC_VER
        name = name.substr(6);
#else
        name = name.substr(name.find_first_not_of("0123456789"));
#endif
        name = name.substr(0, name.size()-2);
        log_entries.push_back({name, std::to_string(var->rawdata)});
    }
    template<typename T>
    void read(ETCS_variable_custom<T> *var)
    {
        T value=0;
        int count=var->size;
        while(count-->0) {
            if ((bits.size()<<3) <= position) {
                error = true;
                return;
            }
            value = value<<1 | operator[](position++);
        }
        var->rawdata = value;
        if (!var->is_valid(m_version))
            sparefound = true;
        log(var);
    }
    template<typename T>
    void peek(ETCS_variable_custom<T> *var, int offset=0)
    {
        int position = this->position+offset;
        T value=0;
        int count=var->size;
        while(count-->0) {
            if ((bits.size()<<3) <= position) {
                return;
            }
            value = value<<1 | operator[](position++);
        }
        var->rawdata = value;
    }
    template<typename T>
    void write(ETCS_variable_custom<T> *var)
    {
        T value=var->rawdata;
        int count=var->size;
        while(count-->0) {
            bool bit = (value>>count)&1;
            if ((position & 7) == 0)
                bits.push_back(bit<<7);
            else if (bit)
                bits[position>>3] |= 1<<(7 - (position&7));
            ++position;
        }
        log(var);
    }
    template<typename T>
    void replace(ETCS_variable_custom<T> *var, int pos)
    {
        if (var->size + pos > (bits.size()<<3)) return;
        for (int i=0; i<var->size; i++) {
            int off = 7-(pos&7);
            int b = pos>>3;
            bits[b] ^= ((bits[b]>>off)&1)<<off;
            bits[b] |= ((var->rawdata>>(var->size-i-1)) & 1)<<off;
            ++pos;
        }
    }
    std::string to_base64();
};