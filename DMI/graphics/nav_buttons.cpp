#include "nav_buttons.h"
#include "button.h"
#include "window.h"
#include "algorithm"
extern vector<window*> active_windows;
extern window menu_main;
extern window navigation_bar;
extern window PASP;
Button main_button(60, 50, mainbut_display, mainbut_pressed);
void mainbut_display()
{
    main_button.setText("Main",12,White);
}
void mainbut_pressed()
{
    active_windows.push_back(&menu_main);
    remove(active_windows.begin(), active_windows.end(), &navigation_bar);
    //remove(active_windows.begin(), active_windows.end(), &PASP);
}
Button override_button(60, 50, overridebut_display, overridebut_pressed);
void overridebut_display()
{
    override_button.setText("Override",12,White);
}
void overridebut_pressed()
{
    
}
Button dataview_button(60, 50, dataviewbut_display, dataviewbut_pressed);
void dataviewbut_display()
{
    dataview_button.setText("Dataview",12,White);
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
}
Button config_button(60, 50, configbut_display, configbut_pressed);
void configbut_display()
{
    config_button.setBackgroundImage("symbols/Setting/SE_04.bmp");
}
void configbut_pressed()
{
}