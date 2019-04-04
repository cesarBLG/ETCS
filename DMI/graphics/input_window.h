#ifndef _INPUT_WINDOWS_H
#define _INPUT_WINDOWS_H
#include "text_button.h"
#include "subwindow.h"
class input_window : public subwindow
{
    public:
    input_window(const char *name);
    protected:
    Button* buttons[12];
};
#endif