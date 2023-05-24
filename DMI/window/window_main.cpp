/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "../graphics/display.h"
#include "../graphics/button.h"
#include "../graphics/icon_button.h"
#include "../graphics/text_button.h"
#include "../STM/stm_objects.h"
void construct_main(window *w, bool custom)
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
    extern IconButton upArrow;
    extern IconButton downArrow;

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
    w->addToLayout(&Y, new RelativeAlignment(nullptr, 0, 465));
    w->addToLayout(&csg, new RelativeAlignment(nullptr, 54, 15));
    w->addToLayout(&a2, new RelativeAlignment(nullptr, 0, 69));
    w->addToLayout(&a23, new RelativeAlignment(nullptr, 0, 69));
    w->addToLayout(&distanceBar, new RelativeAlignment(nullptr, 0, 99));
    w->addToLayout(&a4, new RelativeAlignment(nullptr, 0, 290));
    w->addToLayout(&levelRegion, new RelativeAlignment(nullptr, 0, 315));
    w->addToLayout(&c9, new RelativeAlignment(nullptr, 0, 340));
    w->addToLayout(&e1, new RelativeAlignment(nullptr, 0, 365));
    w->addToLayout(&Y, new ConsecutiveAlignment(&e4, DOWN | LEFT));
    w->addToLayout(&c1, new RelativeAlignment(nullptr, 165, 315));
    w->addToLayout(&c7, new RelativeAlignment(nullptr, 297, 315));
    w->addToLayout(&modeRegion, new RelativeAlignment(&csg, 254, 274, -2));
    w->addToLayout(&releaseRegion, new RelativeAlignment(&csg, 26, 274, -2));
    w->addToLayout(&textArea, new RelativeAlignment(nullptr, 54, 365));
    w->addToLayout(&upArrow, new ConsecutiveAlignment(&textArea, UP|RIGHT, 0));
    w->addToLayout(&downArrow, new ConsecutiveAlignment(&upArrow, DOWN, 0));

    if (!custom)
    {
        w->addToLayout(&a1, new ConsecutiveAlignment(&Z, DOWN | LEFT));
        w->addToLayout(&c234, new ConsecutiveAlignment(&csg, DOWN|LEFT));
        w->addToLayout(&c2, new ConsecutiveAlignment(&csg, DOWN|LEFT));
        w->addToLayout(&c3, new ConsecutiveAlignment(&c2, RIGHT));
        w->addToLayout(&c4, new ConsecutiveAlignment(&c3, RIGHT));
        w->addToLayout(&c5, new ConsecutiveAlignment(&c1, RIGHT));
        w->addToLayout(&c6, new ConsecutiveAlignment(&c5, RIGHT));
        w->addToLayout(&e2, new RelativeAlignment(nullptr, 0, 390));
        w->addToLayout(&e3, new ConsecutiveAlignment(&e2, DOWN));
        w->addToLayout(&e4, new ConsecutiveAlignment(&e3, DOWN));
        w->addToLayout(&b4, new RelativeAlignment(&csg, 140, 274, -2));
        w->addToLayout(&b3, new ConsecutiveAlignment(&b4, LEFT, -2));
        w->addToLayout(&b5, new ConsecutiveAlignment(&b4, RIGHT, -2));
    }
    
    c2.dispBorder = false;
    c3.dispBorder = false;
    c4.dispBorder = false;
    a2.dispBorder = false;
    distanceBar.dispBorder = false;
    csg.touch_left = a1.sx;
    c9.touch_down = e1.sy;
    c9.touch_up = levelRegion.sy;
}
window etcs_default_window([](window *w) { construct_main(w, false); });