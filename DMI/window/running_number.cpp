#include "running_number.h"
#include "../monitor.h"
#include "keyboard.h"
trn_window::trn_window() : input_window("Train running number", 1)
{
    inputs[0] = new trn_input();
    create();
}
trn_input::trn_input()
{
    data = to_string(trn);
    keys = getNumericKeyboard(data_set, data_get);
}
void trn_input::validate()
{
    if(data.size()>6) return;
    trn = stoi(data);
}