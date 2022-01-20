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
#ifndef _CONTROL_H
#define _CONTROL_H
#include <string>
#include "../../EVC/Parser/nlohmann/json.hpp"
using json = nlohmann::json;
/*enum som_step
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
extern bool som_active;
extern som_step som_status;
class window;
class subwindow;
extern std::mutex window_mtx;
extern std::condition_variable window_cv;
void manage_windows();
void right_menu(subwindow *w);
void exit_all();
void exit(subwindow *w);
void set_procedure(std::function<void()> proc);
void start_dialog();*/
void startWindows();
void setWindow(json &j);
#endif
