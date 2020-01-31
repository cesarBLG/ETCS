#pragma once
#include <stdint.h>
#include <vector>
#include "variables.h"
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