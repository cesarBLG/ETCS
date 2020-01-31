#ifndef _KEYBOARD_H
#define _KEYBOARD_H
#include <vector>
#include <string>
#include "../graphics/button.h"
#include "input_data.h"
using namespace std;
vector<Button*> getNumericKeyboard(input_data *input);
vector<Button*> getAlphaNumericKeyboard(input_data *input);
vector<Button*> getSingleChoiceKeyboard(vector<string> posibilities, input_data *input);
#endif