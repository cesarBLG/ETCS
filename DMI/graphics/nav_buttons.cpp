#include "nav_buttons.h"
#include "button.h"
#include "window.h"
#include <algorithm>
#include "display.h"
#include "menu_main.h"
#include "menu_override.h"
#include "menu_settings.h"
#include "menu_spec.h"
Button main_button(60, 50, mainbut_display, mainbut_pressed);
void mainbut_display()
{
    main_button.setText("Main",12,White);
}
void mainbut_pressed()
{
    active_windows.insert(new menu_main());
    active_windows.erase(&navigation_bar);
    active_windows.erase(&PASP);
}
Button override_button(60, 50, overridebut_display, overridebut_pressed);
void overridebut_display()
{
    override_button.setText("Over-\nride",12,White);
}
void overridebut_pressed()
{
    active_windows.insert(new menu_override());
    active_windows.erase(&navigation_bar);
    active_windows.erase(&PASP);
}
Button dataview_button(60, 50, dataviewbut_display, dataviewbut_pressed);
void dataviewbut_display()
{
    dataview_button.setText("Data\nview",12,White);
}
void dataviewbut_pressed()
{

}
Button special_button(60, 50, specbut_display, specbut_pressed);
void specbut_display()
{
    special_button.setText("Spec",12,White);
}
void specbut_pressed()
{
    active_windows.insert(new menu_spec());
    active_windows.erase(&navigation_bar);
    active_windows.erase(&PASP);
}
Button config_button(60, 50, configbut_display, configbut_pressed);
void configbut_display()
{
    config_button.setBackgroundImage("symbols/Setting/SE_04.bmp");
}
void configbut_pressed()
{
    active_windows.insert(new menu_settings());
    active_windows.erase(&navigation_bar);
    active_windows.erase(&PASP);
}