/*
 * European Train Control System
 * Copyright (C) 2019  Iván Izquierdo
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
#include "menu.h"
menu::menu(const char *title) : subwindow(title)
{
    for(int i=0; i<8; i++)
    {
        buttons[i] = nullptr;
    }
}
menu::~menu()
{
    for(int i=0; i<8; i++)
    {
        if(buttons[i]!=nullptr) delete buttons[i];
    }
}
void menu::setLayout()
{
    subwindow::setLayout();
    addToLayout(buttons[0], new RelativeAlignment(nullptr, 334, 45,0));
    addToLayout(buttons[1], new ConsecutiveAlignment(buttons[0],RIGHT,0));
    addToLayout(buttons[2], new ConsecutiveAlignment(buttons[0],DOWN,0));
    addToLayout(buttons[3], new ConsecutiveAlignment(buttons[2],RIGHT,0));
    addToLayout(buttons[4], new ConsecutiveAlignment(buttons[2],DOWN,0));
    addToLayout(buttons[5], new ConsecutiveAlignment(buttons[4],RIGHT,0));
    addToLayout(buttons[6], new ConsecutiveAlignment(buttons[4],DOWN,0));
    addToLayout(buttons[7], new ConsecutiveAlignment(buttons[6],RIGHT,0));
}