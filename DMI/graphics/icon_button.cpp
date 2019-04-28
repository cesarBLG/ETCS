#include "icon_button.h"
IconButton::IconButton(string enabled_path, float sx, float sy, function<void()> pressed, string disabled_path)
{
    setEnabledImage(enabled_path);
    setDisabledImage(disabled_path);
    setSize(sx, sy);
    setPressedAction(pressed);
}
void IconButton::setEnabledImage(string path)
{
    if(path=="") return;
    enabled_image = getImage(path);
}
void IconButton::setDisabledImage(string path)
{
    if(path=="") return;
    disabled_image = getImage(path);
}
void IconButton::paint()
{
    Button::paint();
    if(enabled || disabled_image == nullptr) draw(enabled_image);
    else draw(disabled_image);
}
IconButton::~IconButton()
{
    if(enabled_image != nullptr) delete enabled_image;
    if(disabled_image != nullptr) delete disabled_image;
}