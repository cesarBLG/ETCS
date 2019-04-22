#include "driver_id.h"
#include "window.h"
#include "algorithm"
#include "../monitor.h"
#include "running_number.h"
#include "menu_settings.h"
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
    setLayout();
}
void driver_window::setLayout()
{
    addToLayout(&TRN, new RelativeAlignment(nullptr, 334+142,400+15,0));
    addToLayout(&settings, new ConsecutiveAlignment(&TRN, RIGHT,0));
    input_window::setLayout();
}
void driverid_input::validate()
{
    if(data.size()>8) return;
    driverid = stoul(data);
}
driverid_input::driverid_input()
{
    data = to_string(driverid);
    for(int i=0; i<9; i++)
    {
        keys.push_back(new Button(102, 50, nullptr, nullptr));
    }
    keys[0]->setDisplayFunction([this]
    {
        keys[0]->drawText("1",0,0,0,0,12);
    });
    keys[1]->setDisplayFunction([this]
    {
        keys[1]->drawText("2",-15,0,0,0,12);
        keys[1]->drawText("abc",4,0,0,0,10);
    });
    keys[2]->setDisplayFunction([this]
    {
        keys[2]->drawText("3",-15,0,0,0,12);
        keys[2]->drawText("def",4,0,0,0,10);
    });
    keys[3]->setDisplayFunction([this]
    {
        keys[3]->drawText("4",-15,0,0,0,12);
        keys[3]->drawText("ghi",4,0,0,0,10);
    });
    keys[4]->setDisplayFunction([this]
    {
        keys[4]->drawText("5",-15,0,0,0,12);
        keys[4]->drawText("jkl",4,0,0,0,10);
    });
    keys[5]->setDisplayFunction([this]
    {
        keys[5]->drawText("6",-15,0,0,0,12);
        keys[5]->drawText("mno",10,0,0,0,10);
    });
    keys[6]->setDisplayFunction([this]
    {
        keys[6]->drawText("7",-15,0,0,0,12);
        keys[6]->drawText("pqrs",10,0,0,0,10);
    });
    keys[7]->setDisplayFunction([this]
    {
        keys[7]->drawText("8",-15,0,0,0,12);
        keys[7]->drawText("tuv",4,0,0,0,10);
    });
    keys[8]->setDisplayFunction([this]
    {
        keys[8]->drawText("9",-15,0,0,0,12);
        keys[8]->drawText("wxyz",10,0,0,0,10);
    });
    keys.push_back(new TextButton("DEL", 102, 50, nullptr));
    keys.push_back(new TextButton("0", 102, 50, nullptr));
    keys.push_back(new TextButton(".", 102, 50, nullptr));
    for(int i=0; i<12; i++)
    {
        keys[i]->setPressedAction([this, i]
        {
            if(i<11 && data=="0") data = "";
            if(i<9) data = data + to_string(i+1);
            if(i==9) data = data.substr(0, data.size()-1);
            if(i==10) data = data + "0";
        });
    }
}