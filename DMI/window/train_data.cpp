/*
 * European Train Control System
 * Copyright (C) 2019  Iván Izquierdo
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "train_data.h"
#include "fixed_train_data.h"
#include "keyboard.h"
#include "../monitor.h"
#include "running_number.h"
#include "../tcp/server.h"
train_data_window::train_data_window() : input_window("Train Data", 6, true), SelectType("Select\ntype",60,50)
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
        write_command("SelectType", "");
    });
    create();
}
void train_data_window::setLayout()
{
    input_window::setLayout();
    addToLayout(&SelectType, new RelativeAlignment(&exit_button, 246+30,25,0));
}
train_data_validation_window::train_data_validation_window(std::vector<input_data*> data) : validation_window("Validate train data", data)
{

}