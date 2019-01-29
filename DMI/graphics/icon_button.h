#ifndef _ICON_BUTTON_H
#define _ICON_BUTTON_H
#include "button.h"
class IconButton : public Button
{
    char *path;
    public:
    IconButton(char *path, float sx, float sy, function<void()> pressed = nullptr);
};
#endif
