#include "running_number.h"
#include "../monitor.h"
#include "keyboard.h"
trn_window::trn_window() : input_window("Train running number", 1)
{
    inputs[0] = new trn_input();
    create();
}
void trn_window::sendInformation()
{
    trn = stoi(inputs[0]->getData());
}
trn_input::trn_input()
{
    if (trn != 0)
    {
        data = to_string(trn);
        accepted = true;
    }
    keys = getNumericKeyboard(this);
}
void trn_input::validate()
{
    if(data.size()>6) return;
    valid = true;
}