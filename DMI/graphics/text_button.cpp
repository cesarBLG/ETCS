#include "text_button.h"
TextButton::TextButton(char *text, float sx, float sy, function<void()> pressed)
{
    setSize(sx, sy);
    caption = text;
    setPressedAction(pressed);
    setDisplayFunction([this] {this->setText(this->caption, 12, White);});
}