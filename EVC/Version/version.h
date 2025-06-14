/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <set>
#define VERSION_X(ver) ((ver)>>4)
#define VERSION_Y(ver) ((ver)&15)
extern int operated_version;
extern std::set<int> supported_versions;
bool is_version_supported(int version);
void operate_version(int version, bool rbc);
void load_version();