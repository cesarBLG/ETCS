#ifndef _SUBWINDOW_H
#define _SUBWINDOW_H
#include "window.h"
#include "display.h"
#include "icon_button.h"
class subwindow : public window
{
    IconButton exit_button;
    string title;
    Component title_bar;
    public:
    subwindow(const char *title);
};
#endif