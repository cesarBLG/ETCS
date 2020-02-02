#include "subwindow.h"
subwindow::subwindow(string title, bool full) : window(), fullscreen(full), exit_button("symbols/Navigation/NA_11.bmp", 82, 50), title_bar(full ? 334 : 306,24)
{
    title_bar.addText(title,4,0,12,White, full ? RIGHT : LEFT);
    if (full) title_bar.setBackgroundColor(Black);
    exit_button.setPressedAction([this] 
    {
        exit(this);
    });
}
void subwindow::setLayout()
{
    extern Button downArrow;
    if(fullscreen) addToLayout(&title_bar, new RelativeAlignment(nullptr,0,15,0));
    else addToLayout(&title_bar, new RelativeAlignment(nullptr,334,15,0));
    addToLayout(&exit_button, new ConsecutiveAlignment(&downArrow,RIGHT,0));
}