#include "icon_button.h"
IconButton::IconButton(const char *path, float sx, float sy, function<void()> pressed)
{
    this->path = path;
    setSize(sx, sy);
    setPressedAction(pressed);
    setDisplayFunction([this] {this->setBackgroundImage(this->path);});
}