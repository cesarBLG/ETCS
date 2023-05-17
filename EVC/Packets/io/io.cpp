/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "../types.h"
#include "base64.h"
bit_manipulator::bit_manipulator(std::string base64) : position(0)
{
    write_mode = false;
    std::string str = base64_decode(base64);
    bits.insert(bits.end(), str.begin(), str.end());
}
std::string bit_manipulator::to_base64()
{
    return base64_encode(&bits[0], bits.size());
}