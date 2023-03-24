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
#include "driver_id.h"
#include "window.h"
#include "algorithm"
#include "../monitor.h"
#include "menu_settings.h"
#include "keyboard.h"
#include "../tcp/server.h"
driver_window::driver_window(std::string title, bool show_trn) : input_window(title, 1, false), TRN(get_text("TRN"),82,50), settings("symbols/Setting/SE_04.bmp",82,50)
{
    TRN.setPressedAction([this] 
    {
        write_command("TrainRunningNumber","");
    });
    settings.setPressedAction([this] 
    {
        write_command("navButton","settings");
    });
}
void driver_window::setLayout()
{
    input_window::setLayout();
    addToLayout(&TRN, new RelativeAlignment(nullptr, 334+142,400+15,0));
    addToLayout(&settings, new ConsecutiveAlignment(&TRN, RIGHT,0));
}