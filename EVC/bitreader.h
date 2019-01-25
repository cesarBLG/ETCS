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

#ifndef _BITREADER_H
#define _BITREADER_H
#include "stdint.h"
/**
 * @todo write docs
 */
typedef unsigned long bit_t;
typedef uint8_t byte;
class BitReader
{
    byte* buff;
    bit_t offset = 0;
public:
    BitReader(byte *buff) : buff(buff){}
    bool read();
    bit_t read(bit_t count);
    void putByte(byte b);
    void put(bit_t data, bit_t count);
    void putbit(bool b);
    byte peekByte();
    bit_t readBits(byte *data, bit_t bitoffset, bit_t bitcount);
};

#endif // _BITREADER_H
