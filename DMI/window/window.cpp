/*
 * European Train Control System
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "window.h"
#include "../graphics/button.h"
#include "../sound/sound.h"
#include <chrono>
bool isInside(Component *comp, float x, float y)
{
    return (comp->x-comp->touch_left)<x && (comp->x + comp->sx + comp->touch_right)>x
        && (comp->y-comp->touch_up)<y && (comp->y + comp->sy+comp->touch_down)>y;
}
void window::event(int evNo, float x, float y)
{
    int64_t CurrentTime = (std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::system_clock::now().time_since_epoch())).count();
    bool pressed = evNo == 1;
    std::vector<LayoutElement>& el = getLayoutElements();
    Component *validButtonPressed = nullptr;
	if (pressedButton != nullptr)
	{
		if (!pressedButton->isSensitive() || (pressedButton->isButton && !((Button*)pressedButton)->enabled))
		{
			if (pressedButton->isButton) ((Button*)pressedButton)->pressed = false;
			pressedButton = nullptr;
		}
		else if (isInside(pressedButton, x, y))
		{
			if (pressedButton->upType)
			{
				if (pressedButton->delayType && pressedButton->firstPressedTime + 2000 > CurrentTime)
				{
					if (pressedButton->isButton) ((Button*)pressedButton)->pressed = ((int)((CurrentTime - pressedButton->firstPressedTime) * 4 / 1000)) % 2 == 0;
				}
				else
				{
					if (pressedButton->isButton) ((Button*)pressedButton)->pressed = true;
					if (!pressed)
					{
						validButtonPressed = pressedButton;
					}
				}
			}
			else
			{
				if (pressedButton->isButton)
				{
					((Button*)pressedButton)->pressed = CurrentTime - pressedButton->firstPressedTime < 150 || CurrentTime - pressedButton->lastPressedTime < 150;
				}
				if (pressedButton->firstPressedTime + 1500 < CurrentTime)
				{
					if (pressedButton->lastPressedTime  + 300 < CurrentTime)
					{
						validButtonPressed = pressedButton;
                        playClick();
					    if (pressedButton->isButton) ((Button*)pressedButton)->pressed = true;
						pressedButton->lastPressedTime = CurrentTime;
					}
				}
			}
		}
		else
		{
			pressedButton->firstPressedTime = CurrentTime;
			if (pressedButton->isButton) ((Button*)pressedButton)->pressed = false;
		}
	}
	else if (pressed)
	{
        for(int i=0; i<el.size(); i++)
        {
            Component *comp = el[i].comp;
			if (!comp->isSensitive())
				continue;
			if (comp->isButton) ((Button*)comp)->pressed = false;
			if (isInside(comp, x, y) && (!comp->isButton || ((Button*)comp)->enabled))
			{
				pressedButton = comp;
                playClick();
			    if (pressedButton->isButton) ((Button*)pressedButton)->pressed = true;
				pressedButton->firstPressedTime = CurrentTime;
				if (!comp->upType && (!comp->isButton || ((Button*)comp)->enabled))
					validButtonPressed = pressedButton;
				break;
			}
		}
	}
	if (!pressed && pressedButton != nullptr)
	{
		if (pressedButton->isButton) ((Button*)pressedButton)->pressed = false;
		pressedButton = nullptr;
	}
	if (validButtonPressed!=nullptr) validButtonPressed->setPressed();
}
void window::display()
{
    l->update();
}
window::~window()
{
    delete l;
}
