#include "train_data.h"
#include "fixed_train_data.h"
#include "keyboard.h"
#include "../tcp/server.h"
train_data_window::train_data_window() : input_window("Train Data", 6), SelectType("Select\ntype",60,50)
{
    inputs[0] = new input_data("Length (m)");
    inputs[1] = new input_data("Brake percentage");
    inputs[2] = new input_data("Max speed (km/h)");
    inputs[3] = new input_data("Loading gauge");
    inputs[4] = new input_data("Train category");
    inputs[5] = new input_data("Axle load category");
    for(int i=0; i<3; i++)
    {
        inputs[i]->keys = getNumericKeyboard(inputs[i]);
    }
    inputs[3]->keys = getSingleChoiceKeyboard({"G1", "GA", "GB", "GC", "Out of GC"}, inputs[3]);
    inputs[4]->keys = getSingleChoiceKeyboard({"PASS 1","PASS 2","PASS 3",
        "TILT 1","TILT 2","TILT 3","TILT 4","TILT 5","TILT 6","TILT 7",
        "FP 1","FP2","FP 3","FP 4","FG 1","FG 2","FG 3","FG 4"}, inputs[4]);
    inputs[5]->keys = getSingleChoiceKeyboard({"A","HS17","B1","B2","C2","C3","C4","D2","D3","D4","D4XL","E4","E5"}, inputs[5]);
    SelectType.setPressedAction([this]() {
        exit(this);
        right_menu(new fixed_train_data_window());
    });
    create();
}
void train_data_window::sendInformation()
{
    write_command("setLtrain", inputs[0]->getData());
    write_command("setBrakePercentage", inputs[1]->getData());
    write_command("setVtrain", inputs[2]->getData());
    string str = inputs[4]->getData();
    int cant;
    int cat;
    if (str == "PASS1") {
        cant = 80;
        cat = 2;
    } else if (str == "PASS2") {
        cant = 130;
        cat = 2;
    } else if (str == "PASS3") {
        cant = 150;
        cat = 2;
    } else if (str == "TILT1") {
        cant = 165;
        cat = 2;
    } else if (str == "TILT2") {
        cant = 180;
        cat = 2;
    } else if (str == "TILT3") {
        cant = 210;
        cat = 2;
    } else if (str == "TILT4") {
        cant = 225;
        cat = 2;
    } else if (str == "TILT5") {
        cant = 245;
        cat = 2;
    } else if (str == "TILT6") {
        cant = 275;
        cat = 2;
    } else if (str == "TILT7") {
        cant = 300;
        cat = 2;
    } else if (str == "FP1") {
        cant = 80;
        cat = 0;
    } else if (str == "FP2") {
        cant = 100;
        cat = 0;
    } else if (str == "FP3") {
        cant = 130;
        cat = 0;
    } else if (str == "FP4") {
        cant = 150;
        cat = 0;
    } else if (str == "FG1") {
        cant = 80;
        cat = 1;
    } else if (str == "FG2") {
        cant = 100;
        cat = 1;
    } else if (str == "FG3") {
        cant = 130;
        cat = 1;
    } else if (str == "FG4") {
        cant = 150;
        cat = 1;
    }
    write_command("setTrainCategory", to_string(cat));
    write_command("setCantDeficiency", to_string(cant));
}
void train_data_window::setLayout()
{
    input_window::setLayout();
    addToLayout(&SelectType, new RelativeAlignment(&exit_button, 246+30,25,0));
}