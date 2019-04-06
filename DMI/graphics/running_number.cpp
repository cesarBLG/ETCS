#include "running_number.h"
#include "../monitor.h"
trn_window::trn_window() : input_window("Train running number")
{
    data = to_string(trn);
    buttons[0] = new TextButton("1", 102, 50);
    buttons[1] = new TextButton("2", 102, 50);
    buttons[2] = new TextButton("3", 102, 50);
    buttons[3] = new TextButton("4", 102, 50);
    buttons[4] = new TextButton("5", 102, 50);
    buttons[5] = new TextButton("6", 102, 50);
    buttons[6] = new TextButton("7", 102, 50);
    buttons[7] = new TextButton("8", 102, 50);
    buttons[8] = new TextButton("9", 102, 50);
    buttons[9] = new TextButton("DEL", 102, 50);
    buttons[10] = new TextButton("0", 102, 50);
    buttons[11] = new TextButton(".", 102, 50);
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
void trn_window::validate(string dat)
{
    if(dat.size()>6) return;
    trn = stoi(dat);
}