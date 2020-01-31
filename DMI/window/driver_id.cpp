#include "driver_id.h"
#include "window.h"
#include "algorithm"
#include "../monitor.h"
#include "running_number.h"
#include "menu_settings.h"
#include "keyboard.h"
driver_window::driver_window() : input_window("Driver ID", 1), TRN("TRN",82,50), settings("symbols/Setting/SE_04.bmp",82,50)
{
    inputs[0] = new driverid_input();
    TRN.setPressedAction([this] 
    {
        right_menu(new trn_window());
    });
    settings.setPressedAction([this] 
    {
        right_menu(new menu_settings());
    });
    create();
}
void driver_window::setLayout()
{
    input_window::setLayout();
    addToLayout(&TRN, new RelativeAlignment(nullptr, 334+142,400+15,0));
    addToLayout(&settings, new ConsecutiveAlignment(&TRN, RIGHT,0));
}
void driver_window::sendInformation()
{
    driverid = stoul(inputs[0]->getData());
}
void driverid_input::validate()
{
    if(data.size()>8 || data.size()<1) return;
    unsigned long id = stoul(data);
    if(id!=0) setAccepted(true);
}
driverid_input::driverid_input()
{
    if(driverid!=0)
    {
        data = to_string(driverid);
        accepted = true;
    } 
    keys = getAlphaNumericKeyboard(this);
}