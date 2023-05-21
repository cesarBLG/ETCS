/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "nav_buttons.h"
#include "../graphics/button.h"
#include "../graphics/text_button.h"
#include "../graphics/icon_button.h"
#include "window.h"
#include <algorithm>
#include "../graphics/display.h"
#include "../state/time_hour.h"
#include "../state/gps_pos.h"
#include "../tcp/server.h"
TextButton main_button(get_text_context("Navigation bar", "Main"), 60, 50, mainbut_pressed);
void mainbut_pressed()
{
    write_command("navButton","main");
    //right_menu(new menu_main());
}
TextButton override_button(get_text_context("Navigation bar", "Over-\nride"), 60, 50, overridebut_pressed);
void overridebut_pressed()
{
    write_command("navButton","override");
    //right_menu(new menu_override());
}
TextButton dataview_button(get_text_context("Navigation bar", "Data\nview"), 60, 50, dataviewbut_pressed);
void dataviewbut_pressed()
{
    write_command("navButton","data_view");
    //right_menu(new data_view_window());
}
TextButton special_button(get_text_context("Navigation bar", "Spec"), 60, 50, specbut_pressed);
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