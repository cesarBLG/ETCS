#include "train_data.h"
#include "keyboard.h"
train_data_window::train_data_window() : input_window("Train Data", 4)
{
    inputs[0] = new input_data("Length (m)");
    inputs[1] = new input_data("Brake percentage");
    inputs[2] = new input_data("Max speed (km/h)");
    inputs[3] = new input_data("Axle load category");
    for(int i=0; i<3; i++)
    {
        inputs[i]->keys = getNumericKeyboard([this,i](string s) {inputs[i]->data = s;}, [this,i]{return inputs[i]->data;});
    }
    confirmation_label.setDisplayFunction([this]{confirmation_label.setText("Train data entry complete?", 12, White);});
    setLayout();
}
