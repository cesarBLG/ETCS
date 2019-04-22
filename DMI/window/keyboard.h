#ifndef _KEYBOARD_H
#define _KEYBOARD_H
#include <vector>
#include <string>
#include "../graphics/button.h"
using namespace std;
vector<Button*> getNumericKeyboard(function<void(string)> setData, function<string()> getData);
#endif