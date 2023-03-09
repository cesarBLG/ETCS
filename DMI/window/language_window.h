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
#ifndef _LANGUAGE_WINDOW_H
#define _LANGUAGE_WINDOW_H
#include "data_entry.h"
#include "data_validation.h"
#include "../monitor.h"
#include <vector>
class language_window : public input_window
{
    public:
    language_window(std::string lang, std::vector<std::string> languages);
    void sendInformation() override;
};
class language_input : public input_data
{
    public:
    language_input(std::string lang, std::vector<std::string> languages);
    void validate() override;
};
#endif