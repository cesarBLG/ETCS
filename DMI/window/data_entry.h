#ifndef _DATA_ENTRY_H
#define _DATA_ENTRY_H
#include "subwindow.h"
#include "input_data.h"
#include <map>
class input_window : public subwindow
{
    IconButton prev_button;
    IconButton next_button;
    TextButton button_yes;
    int cursor=0;
    int nfields;
    protected:
    Component confirmation_label;
    map<int, input_data*> inputs;
    Button* buttons[12];
    public:
    input_window(const char *name, int nfields);
    void setLayout() override;
    virtual ~input_window();
};
#endif