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
#include "nav_buttons.h"
#include "../graphics/button.h"
#include "window.h"
#include <algorithm>
#include "../graphics/display.h"
#include "menu_main.h"
#include "menu_override.h"
#include "menu_settings.h"
#include "menu_spec.h"
#include "data_view.h"
#include "../state/time_hour.h"
#include "../state/gps_pos.h"
#include "../tcp/server.h"
TextButton main_button(get_text("Main"), 60, 50, mainbut_pressed);
void mainbut_pressed()
{
    write_command("navButton","main");
    //right_menu(new menu_main());
}
TextButton override_button(get_text("Over-\nride"), 60, 50, overridebut_pressed);
void overridebut_pressed()
{
    write_command("navButton","override");
    //right_menu(new menu_override());
}
TextButton dataview_button(get_text("Data\nview"), 60, 50, dataviewbut_pressed);
void dataviewbut_pressed()
{
    write_command("navButton","data_view");
    //right_menu(new data_view_window());
}
TextButton special_button(get_text("Spec"), 60, 50, specbut_pressed);
void specbut_pressed()
{
    write_command("navButton","spec");
    //right_menu(new menu_spec());
}
IconButton config_button("symbols/Setting/SE_04.bmp", 60, 50, configbut_pressed);
void configbut_pressed()
{
    write_command("navButton","settings");
    //right_menu(new menu_settings());
}
void construct_nav(window *w)
{
    w->addToLayout(&main_button, new RelativeAlignment(nullptr, 580, 15, 0));
    w->addToLayout(&override_button, new ConsecutiveAlignment(&main_button,DOWN,0));
    w->addToLayout(&dataview_button, new ConsecutiveAlignment(&override_button,DOWN,0));
    w->addToLayout(&special_button, new ConsecutiveAlignment(&dataview_button,DOWN,0));
    w->addToLayout(&config_button, new ConsecutiveAlignment(&special_button,DOWN,0));
    w->addToLayout(&time_hour, new RelativeAlignment(nullptr,517,415));
    w->addToLayout(&gps_pos, new ConsecutiveAlignment(&time_hour, LEFT));
}
window navigation_bar(construct_nav);