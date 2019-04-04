#ifndef _MENU_H
#define _MENU_H
#include "subwindow.h"
#include "display.h"
#include "icon_button.h"
class menu : public subwindow
{
    protected:
    Button* buttons[8];
    public:
    menu(char *title);
    ~menu();
    void setLayout();
};
#endif