#include "running_number.h"
#include "../monitor.h"
#include "keyboard.h"
trn_window::trn_window() : input_window("Train running number", 1)
{
    inputs[0] = new trn_input();
    setLayout();
}
trn_input::trn_input()
{
    data = to_string(trn);
    keys = getNumericKeyboard([this](string d){ data = d;}, [this]{return data;});
}
void trn_input::validate()
{
    if(data.size()>6) return;
    trn = stoi(data);
}