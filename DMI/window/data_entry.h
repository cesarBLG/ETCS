#ifndef _DATA_ENTRY_H
#define _DATA_ENTRY_H
#include "subwindow.h"
class input_window : public subwindow
{
    Component input_field;
    protected:
    string data;
    Button* buttons[12];
    public:
    input_window(const char *name);
    virtual void validate(string dat){};
    void setLayout();
    virtual ~input_window();
};
#endif