#include "display.h"
#include "button.h"
#include "text_button.h"
#include "time_hour.h"
void construct_main();
window main_window(construct_main);
void construct_main()
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
    
    main_window.addToLayout(&Z, new RelativeAlignment(nullptr, 0, 0, 0));
    main_window.addToLayout(&a1, new ConsecutiveAlignment(&Z, DOWN | LEFT));
    main_window.addToLayout(&csg, new ConsecutiveAlignment(&a1, UP | RIGHT));
    main_window.addToLayout(&a2, new ConsecutiveAlignment(&a1, DOWN));
    main_window.addToLayout(&a23, new ConsecutiveAlignment(&a1, DOWN));
    main_window.addToLayout(&distanceBar, new ConsecutiveAlignment(&a2, DOWN));
    a2.dispBorder = false;
    distanceBar.dispBorder = false;
    main_window.addToLayout(&a4, new ConsecutiveAlignment(&distanceBar, DOWN));
    main_window.addToLayout(&levelRegion, new ConsecutiveAlignment(&a4, DOWN));
    main_window.addToLayout(&c9, new ConsecutiveAlignment(&levelRegion, DOWN));
    main_window.addToLayout(&c234, new ConsecutiveAlignment(&csg, DOWN|LEFT));
    main_window.addToLayout(&c2, new ConsecutiveAlignment(&csg, DOWN|LEFT));
    main_window.addToLayout(&c3, new ConsecutiveAlignment(&c2, RIGHT));
    main_window.addToLayout(&c4, new ConsecutiveAlignment(&c3, RIGHT));
    c2.dispBorder = false;
    c3.dispBorder = false;
    c4.dispBorder = false;
    main_window.addToLayout(&c1, new ConsecutiveAlignment(&c4, RIGHT));
    main_window.addToLayout(&c5, new ConsecutiveAlignment(&c1, RIGHT));
    main_window.addToLayout(&c6, new ConsecutiveAlignment(&c5, RIGHT));
    main_window.addToLayout(&c7, new ConsecutiveAlignment(&c6, RIGHT));
    main_window.addToLayout(&textArea, new ConsecutiveAlignment(&c2, DOWN|LEFT));
    main_window.addToLayout(&upArrow, new ConsecutiveAlignment(&textArea, UP|RIGHT, 0));
    main_window.addToLayout(&downArrow, new ConsecutiveAlignment(&upArrow, DOWN, 0));
    main_window.addToLayout(&e1, new ConsecutiveAlignment(&c9, DOWN));
    main_window.addToLayout(&e2, new ConsecutiveAlignment(&e1, DOWN));
    main_window.addToLayout(&e3, new ConsecutiveAlignment(&e2, DOWN));
    main_window.addToLayout(&e4, new ConsecutiveAlignment(&e3, DOWN));
    main_window.addToLayout(&Y, new ConsecutiveAlignment(&e4, DOWN | LEFT));
    main_window.addToLayout(&modeRegion, new RelativeAlignment(&csg, 254, 274, -2));
    main_window.addToLayout(&releaseRegion, new RelativeAlignment(&csg, 26, 274, -2));
    main_window.addToLayout(&b4, new RelativeAlignment(&csg, 140, 274, -2));
    main_window.addToLayout(&b3, new ConsecutiveAlignment(&b4, LEFT, -2));
    main_window.addToLayout(&b5, new ConsecutiveAlignment(&b4, RIGHT, -2));
    main_window.addToLayout(&time_hour, new RelativeAlignment(nullptr,517,415));


    extern bool showSpeeds;
    csg.setPressedAction([]() {showSpeeds = !showSpeeds;});
}