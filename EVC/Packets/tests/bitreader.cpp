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

#include "bitreader.h"
#include <iostream>
using namespace std;
static inline byte getFirst(byte b, int p)
{
    return b>>(8-p);
}
static inline byte getLast(byte b, int p)
{
    return ((b<<(8-p))&255)>>(8-p);
}
static inline int min(int a, int b)
{
    return a<b ? a : b;
}
bit_t BitReader::read(bit_t count)
{
    //size c0 = count;
    bit_t val=0;
    while(count)
    {
        int o = min(count,8);
        val = (val<<o) + (getFirst(peekByte(), o));
        count-=o;
        offset+=o;
    }
    return val;
}
void BitReader::putByte(byte b)
{
    int rest = 8-(offset%8);
    int pos = offset/8;
    buff[pos] += getFirst(b, rest);
    buff[pos+1] = (b<<rest) & 255;
    offset+=8;
}
void BitReader::putbit(bool b)
{
    if(offset%8) buff[offset/8] += 1<<(7-(offset%8));
    else buff[offset/8] = 128;
    offset++;
}
void BitReader::put(bit_t data, bit_t count)
{
    /*int c = count + offset%8;
    if(c%8) c = c/8 + 1;
    else c = c/8;
    data += <<(count+8);
    data = data<<(offset%8)
    memcpy(buff, &data, c);
    offset+=count;*/
}
byte BitReader::peekByte()
{
    byte val = buff[offset/8];
    int extra = offset%8;
    if(!extra) return val;
    return ((val<<extra) & 255) + getFirst(buff[offset/8 + 1], extra);
}
