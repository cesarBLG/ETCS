#ifndef _MENU_H
#define _MENU_H
#include "subwindow.h"
class menu : public subwindow
{
    protected:
    Button* buttons[8];
    public:
    menu(const char *title);
    ~menu();
    void setLayout();
};
#endif