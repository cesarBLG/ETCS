/*
 * European Train Control System
 * Copyright (C) 2022  César Benito <cesarbema2009@hotmail.com>
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
#ifndef _SET_VBC_H
#define _SET_VBC_H
#include "data_entry.h"
#include "data_validation.h"
class set_vbc_window : public input_window
{
    public:
    set_vbc_window();
    void sendInformation() override;
};
class remove_vbc_window : public input_window
{
    public:
    remove_vbc_window();
    void sendInformation() override;
};
#endif