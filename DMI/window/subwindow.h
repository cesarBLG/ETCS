#ifndef _SUBWINDOW_H
#define _SUBWINDOW_H
#include "window.h"
#include "../graphics/display.h"
#include "../graphics/icon_button.h"
#include "../graphics/text_button.h"
class subwindow : public window
{
    IconButton exit_button;
    string title;
    Component title_bar;
    public:
    subwindow(const char *title);
};
#endif