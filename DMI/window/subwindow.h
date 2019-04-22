#ifndef _SUBWINDOW_H
#define _SUBWINDOW_H
#include "window.h"
#include "../graphics/display.h"
#include "../graphics/icon_button.h"
#include "../graphics/text_button.h"
class subwindow : public window
{
    string title;
    bool fullscreen;
    Component title_bar;
    protected:
    IconButton exit_button;
    public:
    subwindow(const char *title, bool full = false);
    virtual void setLayout();
};
#endif