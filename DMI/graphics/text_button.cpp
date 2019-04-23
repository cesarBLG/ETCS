#include "text_button.h"
TextButton::TextButton(string text, float sx, float sy, function<void()> pressed)
{
    setSize(sx, sy);
    caption = text;
    setPressedAction(pressed);
    setDisplayFunction([this] {this->setText(this->caption.c_str(), 12, enabled ? fgColor : DarkGrey);});
}