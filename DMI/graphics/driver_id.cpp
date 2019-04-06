#include "driver_id.h"
#include "window.h"
#include "algorithm"
#include "../monitor.h"
driver_window::driver_window() : input_window("Driver ID")
{
    data = to_string(driverid);
    buttons[0] = new Button(102, 50, nullptr, nullptr);
    buttons[0]->setDisplayFunction([this]
    {
        buttons[0]->drawText("1",0,0,0,0,12);
    });
    buttons[1] = new Button(102, 50, nullptr, nullptr);
    buttons[1]->setDisplayFunction([this]
    {
        buttons[1]->drawText("2",-15,0,0,0,12);
        buttons[1]->drawText("abc",4,0,0,0,10);
    });
    buttons[2] = new Button(102, 50, nullptr, nullptr);
    buttons[2]->setDisplayFunction([this]
    {
        buttons[2]->drawText("3",-15,0,0,0,12);
        buttons[2]->drawText("def",4,0,0,0,10);
    });
    buttons[3] = new Button(102, 50, nullptr, nullptr);
    buttons[3]->setDisplayFunction([this]
    {
        buttons[3]->drawText("4",-15,0,0,0,12);
        buttons[3]->drawText("ghi",4,0,0,0,10);
    });
    buttons[4] = new Button(102, 50, nullptr, nullptr);
    buttons[4]->setDisplayFunction([this]
    {
        buttons[4]->drawText("5",-15,0,0,0,12);
        buttons[4]->drawText("jkl",4,0,0,0,10);
    });
    buttons[5] = new Button(102, 50, nullptr, nullptr);
    buttons[5]->setDisplayFunction([this]
    {
        buttons[5]->drawText("6",-15,0,0,0,12);
        buttons[5]->drawText("mno",10,0,0,0,10);
    });
    buttons[6] = new Button(102, 50, nullptr, nullptr);
    buttons[6]->setDisplayFunction([this]
    {
        buttons[6]->drawText("7",-15,0,0,0,12);
        buttons[6]->drawText("pqrs",10,0,0,0,10);
    });
    buttons[7] = new Button(102, 50, nullptr, nullptr);
    buttons[7]->setDisplayFunction([this]
    {
        buttons[7]->drawText("8",-15,0,0,0,12);
        buttons[7]->drawText("tuv",4,0,0,0,10);
    });
    buttons[8] = new Button(102, 50, nullptr, nullptr);
    buttons[8]->setDisplayFunction([this]
    {
        buttons[8]->drawText("9",-15,0,0,0,12);
        buttons[8]->drawText("wxyz",10,0,0,0,10);
    });
    buttons[9] = new TextButton("DEL", 102, 50, nullptr);
    buttons[10] = new TextButton("0", 102, 50, nullptr);
    buttons[11] = new TextButton(".", 102, 50, nullptr);
    for(int i=0; i<12; i++)
    {
        buttons[i]->setPressedAction([this, i]
        {
            if(i<11 && data=="0") data = "";
            if(i<9) data = data + to_string(i+1);
            if(i==9) data = data.substr(0,data.size()-1);
            if(i==10) data = data + "0";
        });
    }
    setLayout();
}
void driver_window::validate(string dat)
{
    if(dat.size()>8) return;
    driverid = stoul(dat);
}