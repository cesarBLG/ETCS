#include "train_data.h"
#include "fixed_train_data.h"
#include "keyboard.h"
#include "../tcp/server.h"
#include "../../EVC/Parser/nlohmann/json.hpp"
#include <fstream>
using json = nlohmann::json;
fixed_train_data_window::fixed_train_data_window() : input_window("Train Data", 1), SelectType("Select\ntype",60,50)
{
    inputs[0] = new fixed_train_data_input();
    SelectType.setPressedAction([this]() {
        exit(this);
        right_menu(new train_data_window());
    });
    create();
}
void fixed_train_data_window::sendInformation()
{
    write_command("setTrainData", inputs[0]->getData());
}
void fixed_train_data_window::setLayout()
{
    input_window::setLayout();
    addToLayout(&SelectType, new RelativeAlignment(&exit_button, 246+30,25,0));
}
fixed_train_data_input::fixed_train_data_input()
{
    std::ifstream file("../EVC/traindata.txt");
    json j;
    file >> j;
    vector<string> types;
    for (auto it = j.begin(); it!=j.end(); ++it) {
        types.push_back(it.key());
    }
    keys = getSingleChoiceKeyboard(types, this);
}
void fixed_train_data_input::validate()
{
    valid = true;
}