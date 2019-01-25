#ifndef _BUTTON_H
#define _BUTTON_H
#include "component.h"
class Button : public Component
{
    protected:
    Button() {isButton = true;}
    public:
    bool enabled = true;
    virtual void paint();
    void setPressedAction(function<void()> pressed)
    {
        pressedAction = pressed;
    }
    Button(float sx, float sy, function<void()> display, function<void()> pressed) : Component(sx,sy,display)
    {
        isButton = true;
        setPressedAction(pressed);
    }
};
#endif