#include "train_data.h"
#include "keyboard.h"
#include "../tcp/server.h"
train_data_window::train_data_window() : input_window("Train Data", 6)
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
    create();
}
void train_data_window::sendInformation()
{
    write_command("setLtrain", inputs[0]->getData());
    write_command("setBrakePercentage", inputs[1]->getData());
    write_command("setVtrain", inputs[2]->getData());
    write_command("setTrainCategory", inputs[4]->getData());
}