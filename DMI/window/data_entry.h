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
    Button* empty_button[12];
    int cursor=0;
    int nfields;
    protected:
    Component confirmation_label;
    map<int, input_data*> inputs;
    Button* buttons[12];
    virtual void setLayout() override;
    virtual void sendInformation() {}
    void create();
    public:
    input_window(string name, int nfields);
    virtual ~input_window();
};
#endif