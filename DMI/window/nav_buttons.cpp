#include "nav_buttons.h"
#include "../graphics/button.h"
#include "window.h"
#include <algorithm>
#include "../graphics/display.h"
#include "menu_main.h"
#include "menu_override.h"
#include "menu_settings.h"
#include "menu_spec.h"
#include "../state/time_hour.h"
#include "../state/gps_pos.h"
void construct_nav();
window navigation_bar(construct_nav);
TextButton main_button("Main", 60, 50, mainbut_pressed);
void mainbut_pressed()
{
    right_menu(new menu_main());
}
TextButton override_button("Over-\nride", 60, 50, overridebut_pressed);
void overridebut_pressed()
{
    right_menu(new menu_override());
}
TextButton dataview_button("Data\nview", 60, 50, dataviewbut_pressed);
void dataviewbut_pressed()
{

}
TextButton special_button("Spec", 60, 50, specbut_pressed);
void specbut_pressed()
{
    right_menu(new menu_spec());
}
IconButton config_button("symbols/Setting/SE_04.bmp", 60, 50, configbut_pressed);
void configbut_pressed()
{
    right_menu(new menu_settings());
}
void construct_nav()
{
    navigation_bar.addToLayout(&main_button, new RelativeAlignment(nullptr, 580, 15, 0));
    navigation_bar.addToLayout(&override_button, new ConsecutiveAlignment(&main_button,DOWN,0));
    navigation_bar.addToLayout(&dataview_button, new ConsecutiveAlignment(&override_button,DOWN,0));
    navigation_bar.addToLayout(&special_button, new ConsecutiveAlignment(&dataview_button,DOWN,0));
    navigation_bar.addToLayout(&config_button, new ConsecutiveAlignment(&special_button,DOWN,0));
    navigation_bar.addToLayout(&time_hour, new RelativeAlignment(nullptr,517,415));
    navigation_bar.addToLayout(&gps_pos, new ConsecutiveAlignment(&time_hour, LEFT));
}