/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _DMI_H
#define _DMI_H
#include <string>
void start_dmi();
void send_command(std::string command, std::string value);
void set_persistent_command(std::string command, std::string value);
#endif