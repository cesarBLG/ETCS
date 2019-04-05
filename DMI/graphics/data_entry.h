#ifndef _DATA_ENTRY_H
#define _DATA_ENTRY_H
#include "text_button.h"
#include "subwindow.h"
class input_window : public subwindow
{
    Component input_field;
    protected:
    string data;
    Button* buttons[12];
    public:
    input_window(const char *name);
    void setLayout();
    ~input_window();
};
#endif