#ifndef _TEXT_BUTTON_H
#define _TEXT_BUTTON_H
#include "button.h"
class TextButton : public Button
{
    const char *caption;
    public:
    TextButton(const char *text, float sx, float sy, function<void()> pressed);
};
#endif
