#include "subwindow.h"
subwindow::subwindow(const char *title) : window(), exit_button("symbols/Navigation/NA_11.bmp", 82, 50), title_bar(306,24)
{
    extern Button downArrow;
    title_bar.setDisplayFunction([this,title]{title_bar.drawText(title,4,0,0,0,12,White, LEFT);});
    addToLayout(&title_bar, new RelativeAlignment(nullptr,334,15,0));
    addToLayout(&exit_button, new ConsecutiveAlignment(&downArrow,RIGHT,0));
    exit_button.setPressedAction([this] 
    {
        exit(this);
    });
}