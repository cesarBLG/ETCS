#ifndef _ICON_BUTTON_H
#define _ICON_BUTTON_H
#include "button.h"
class IconButton : public Button
{
    image_graphic *enabled_image = nullptr;
    image_graphic *disabled_image = nullptr;
    public:
    void paint() override;
    void setEnabledImage(string path);
    void setDisabledImage(string path);
    IconButton(string enabled_path, float sx, float sy, function<void()> pressed = nullptr, string disabled_path = "");
    ~IconButton();
};
#endif
