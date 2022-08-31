/*
 * European Train Control System
 * Copyright (C) 2019  Iván Izquierdo
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
#ifndef _LEVEL_WINDOW_H
#define _LEVEL_WINDOW_H
#include "data_entry.h"
#include "data_validation.h"
#include "../monitor.h"
#include <vector>
class level_window : public input_window
{
    public:
    level_window(std::string level, std::vector<std::string> levels);
    void sendInformation() override;
};
class level_input : public input_data
{
    public:
    level_input(std::string, std::vector<std::string> levels, bool echo=false);
    void validate() override;
};
class level_validation_window : public validation_window
{
    public:
    level_validation_window(std::string);
};
#endif