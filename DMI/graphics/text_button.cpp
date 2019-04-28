#include "text_button.h"
TextButton::TextButton(string text, float sx, float sy, function<void()> pressed, int size) : size(size), caption(text)
{
    setSize(sx, sy);
    setPressedAction(pressed);
    enabled_text = getText(caption, 0, 0, size, fgColor);
    disabled_text = getText(caption, 0, 0, size, DarkGrey);
}
void TextButton::paint()
{
    Button::paint();
    if(enabled)
    {
        if(enabled_text->color != fgColor)
        {
            delete enabled_text;
            enabled_text = getText(caption, 0, 0, size, fgColor);
        }
        draw(enabled_text);
    }
    else draw(disabled_text);
}
TextButton::~TextButton()
{
    if(enabled_text!=nullptr) delete enabled_text;
    if(disabled_text!=nullptr) delete disabled_text;
}