/*
 * European Train Control System
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
#pragma once
#include "../Supervision/train_data.h"
#include "mode_transition.h"
#include "../Packets/radio.h"
enum som_step
{
    S0,
    S1,
    D2,
    D3,
    D7,
    S2,
    S3,
    S4,
    A29,
    S10,
    S12,
    D12,
    S13,
    D10,
    D11,
    S11,
    S20,
    S21,
    S22,
    S23,
    S24,
    S25,
    A31,
    D31,
    A32,
    D32,
    A33,
    A34,
    D33,
    A35,
    D22,
    A23,
    A24,
    A38,
    A39,
    A40
};
extern som_step som_status;
extern bool som_active;
extern bool ongoing_mission;
void update_SoM();
void start_pressed();