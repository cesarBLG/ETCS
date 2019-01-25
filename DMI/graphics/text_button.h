#ifndef _TEXT_BUTTON_H
#define _TEXT_BUTTON_H
#include "button.h"
class TextButton : public Button
{
    char *caption;
    public:
    TextButton(char *text, float sx, float sy, function<void()> pressed);
};
#endif
