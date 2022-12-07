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
#include "../graphics/display.h"
#include "../graphics/button.h"
#include "../graphics/text_button.h"
void construct_main(window *w)
{
    extern Component csg;
    extern Component a1;
    extern Component a2;
    extern Component a23;
    extern Component distanceBar;
    extern Component a4;
    extern Component levelRegion;
    extern Component c9;
    extern Component c234;
    extern Component c2;
    extern Component c3;
    extern Component c4;
    extern Component c1;
    extern Component c5;
    extern Component c6;
    extern Component c7;
    extern Component textArea;
    extern Button upArrow;
    extern Button downArrow;

    extern Component e1;
    extern Component e2;
    extern Component e3;
    extern Component e4;
    extern Component modeRegion;
    extern Component releaseRegion;
    extern Component b3;
    extern Component b4;
    extern Component b5;
    
    w->addToLayout(&Z, new RelativeAlignment(nullptr, 0, 0, 0));
    w->addToLayout(&a1, new ConsecutiveAlignment(&Z, DOWN | LEFT));
    w->addToLayout(&csg, new ConsecutiveAlignment(&a1, UP | RIGHT));
    w->addToLayout(&a2, new ConsecutiveAlignment(&a1, DOWN));
    w->addToLayout(&a23, new ConsecutiveAlignment(&a1, DOWN));
    w->addToLayout(&distanceBar, new ConsecutiveAlignment(&a2, DOWN));
    a2.dispBorder = false;
    distanceBar.dispBorder = false;
    w->addToLayout(&a4, new ConsecutiveAlignment(&distanceBar, DOWN));
    w->addToLayout(&levelRegion, new ConsecutiveAlignment(&a4, DOWN));
    w->addToLayout(&c9, new ConsecutiveAlignment(&levelRegion, DOWN));
    w->addToLayout(&c234, new ConsecutiveAlignment(&csg, DOWN|LEFT));
    w->addToLayout(&c2, new ConsecutiveAlignment(&csg, DOWN|LEFT));
    w->addToLayout(&c3, new ConsecutiveAlignment(&c2, RIGHT));
    w->addToLayout(&c4, new ConsecutiveAlignment(&c3, RIGHT));
    c2.dispBorder = false;
    c3.dispBorder = false;
    c4.dispBorder = false;
    w->addToLayout(&c1, new ConsecutiveAlignment(&c4, RIGHT));
    w->addToLayout(&c5, new ConsecutiveAlignment(&c1, RIGHT));
    w->addToLayout(&c6, new ConsecutiveAlignment(&c5, RIGHT));
    w->addToLayout(&c7, new ConsecutiveAlignment(&c6, RIGHT));
    w->addToLayout(&textArea, new ConsecutiveAlignment(&c2, DOWN|LEFT));
    w->addToLayout(&upArrow, new ConsecutiveAlignment(&textArea, UP|RIGHT, 0));
    w->addToLayout(&downArrow, new ConsecutiveAlignment(&upArrow, DOWN, 0));
    w->addToLayout(&e1, new ConsecutiveAlignment(&c9, DOWN));
    w->addToLayout(&e2, new ConsecutiveAlignment(&e1, DOWN));
    w->addToLayout(&e3, new ConsecutiveAlignment(&e2, DOWN));
    w->addToLayout(&e4, new ConsecutiveAlignment(&e3, DOWN));
    w->addToLayout(&Y, new ConsecutiveAlignment(&e4, DOWN | LEFT));
    w->addToLayout(&modeRegion, new RelativeAlignment(&csg, 254, 274, -2));
    w->addToLayout(&releaseRegion, new RelativeAlignment(&csg, 26, 274, -2));
    w->addToLayout(&b4, new RelativeAlignment(&csg, 140, 274, -2));
    w->addToLayout(&b3, new ConsecutiveAlignment(&b4, LEFT, -2));
    w->addToLayout(&b5, new ConsecutiveAlignment(&b4, RIGHT, -2));

    csg.touch_left = a1.sx;
    c9.touch_down = e1.sy;
    c9.touch_up = levelRegion.sy;
}
window etcs_default_window(construct_main);