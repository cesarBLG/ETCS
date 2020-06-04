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
#ifndef _FIXED_TRAIN_DATA_WINDOW_H
#define _FIXED_TRAIN_DATA_WINDOW_H
#include "data_entry.h"
class fixed_train_data_window : public input_window
{
    public:
    TextButton SelectType;
    fixed_train_data_window();
    void validate(string dat, int field)
    {

    }
    void sendInformation() override;
    void setLayout() override;
};
class fixed_train_data_input : public input_data
{
    public:
    fixed_train_data_input();
    void validate() override;
};
#endif