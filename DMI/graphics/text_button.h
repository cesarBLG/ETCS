#ifndef _TEXT_BUTTON_H
#define _TEXT_BUTTON_H
#include "button.h"
class TextButton : public Button
{
    string caption;
    text_graphic *enabled_text;
    text_graphic *disabled_text;
    int size;
    public:
    void paint() override;
    TextButton(string text, float sx, float sy, function<void()> pressed = nullptr, int size = 12);
    ~TextButton();
};
#endif
