/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "../graphics/component.h"
#include "platform_runtime.h"
#include "../state/acks.h"
#include <map>
void input_received(UiPlatform::InputEvent ev);
void setupSoftKeys();
class SoftKey
{
    float x;
    float y;
    public:
    SoftKey() = default;
    SoftKey(float x, float y) : x(x), y(y) {}
    void setPressed(bool pressed)
    {
        input_received({pressed ? UiPlatform::InputEvent::Action::Press : UiPlatform::InputEvent::Action::Release, x, y});
    }
};
extern std::map<int,SoftKey> softF;
extern std::map<int,SoftKey> softH;
class HardwareButton
{
    protected:
    Component *attachedComponent;
    public:
    virtual void setPressed(bool pressed)
    {
        if (attachedComponent == nullptr) return;
        input_received({pressed ? UiPlatform::InputEvent::Action::Press : UiPlatform::InputEvent::Action::Release, attachedComponent->x + attachedComponent->sx/2, attachedComponent->y + attachedComponent->sy/2});
    }
};
class ExternalAckButton : public HardwareButton
{
    public:
    void setPressed(bool pressed) override
    {
        attachedComponent = componentAck;
        HardwareButton::setPressed(pressed);
    }
};
extern ExternalAckButton externalAckButton;