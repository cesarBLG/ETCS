#ifndef _TEXT_BUTTON_H
#define _TEXT_BUTTON_H
#include "button.h"
class TextButton : public Button
{
    string caption;
    public:
    TextButton(string text, float sx, float sy, function<void()> pressed = nullptr);
};
#endif
