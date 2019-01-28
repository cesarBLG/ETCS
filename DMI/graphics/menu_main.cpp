#include "menu_main.h"
#include "button.h"
#include "window.h"
#include "algorithm"
extern vector<window*> active_windows;
extern window menu_main;
extern window navigation_bar;
extern window PASP;

Button start_button(153, 50, startbut_display, startbut_pressed);
void startbut_display(){
    start_button.setText("Start", 12, White);
}
void startbut_pressed()
{
}
Button driverid_button(153, 50, driveridbut_display, driveridbut_pulsado);
void driveridbut_display()
{
    driverid_button.setText("Driver ID", 12, White);
}
void driveridbut_pulsado()
{
}
Button level_button(153, 50, levelbut_display, levelbut_pressed);
void levelbut_display()
{
    level_button.setText("Level", 12, White);
}
void levelbut_pressed()
{

}
Button shunting_button(153, 50, shuntbut_display, shuntbut_pressed);
void shuntbut_display()
{
    shunting_button.setText("Shunting", 12, White);
}
void shuntbut_pressed()
{
}
Button maintsh_button(153, 50, maintshbut_display, maintshbut_pressed);
void maintshbut_display()
{
    maintsh_button.setText("Maintain Shunting", 12, DarkGrey);
}
void maintshbut_pressed()
{
}
Button traindata_button(153, 50, traindatabut_display, traindatabut_pressed);
void traindatabut_display()
{
    traindata_button.setText("Train data", 12, White);
}
void traindatabut_pressed()
{
}
Button trainrn_button(153, 50, trnbut_display, trnbut_pressed);
void trnbut_display()
{
    trainrn_button.setText("Train running number", 12, White);
}
void trnbut_pressed()
{
}
Button nl_button(153, 50, nl_display, nl_pressed);
void nl_display()
{
    nl_button.setText("Non-Leading", 12, White);
}
void nl_pressed()
{
}
Button exit_button(82, 50, exitbut_display, exitbut_pressed);
void exitbut_display()
{
    exit_button.setBackgroundImage("symbols/Navigation/NA_11.bmp");
}
void exitbut_pressed()
{
    active_windows.push_back(&navigation_bar);
    active_windows.push_back(&PASP);
    remove(active_windows.begin(), active_windows.end(), &menu_main);
}