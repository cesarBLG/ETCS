#ifndef _MENU_H
#define _MENU_H
#include "window.h"
#include "display.h"
#include "icon_button.h"
class menu : public window
{
    IconButton exit_button;
    protected:
    Button* buttons[8];
    public:
    menu();
    void setLayout();
    ~menu();
};
#endif