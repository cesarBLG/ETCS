#include "display.h"
#include <chrono>
#include <cstdio>
#include "button.h"
#include "text_button.h"
#include "menu.h"
using namespace std;
window main_window;
window navigation_bar;
window PASP;
unordered_set<window*> active_windows;
#include <iostream>
void displayETCS()
{
    auto start = chrono::system_clock::now();
    for(auto it=active_windows.begin(); it!=active_windows.end(); ++it)
    {
        (*it)->display();
    }
    auto end = chrono::system_clock::now();
    chrono::duration<double> diff = end-start;
    //printf("%f\n", diff.count());
}
void prepareLayout()
{
    active_windows.insert(&main_window);
    active_windows.insert(&PASP);

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

    extern Button main_button;
    extern Button override_button;
    extern Button dataview_button;
    extern Button special_button;
    extern Button config_button;

    extern Component e1;
    extern Component e2;
    extern Component e3;
    extern Component e4;
    extern Component modeRegion;
    extern Component releaseRegion;
    extern Component b3;
    extern Component b4;
    extern Component b5;

    // PASP
    extern Component distancePASP;
    extern IconButton zoomin;
    extern IconButton zoomout;
    PASP.addToLayout(&distancePASP, new RelativeAlignment(nullptr, 334,15));
    PASP.addToLayout(&zoomin, new RelativeAlignment(&distancePASP, 20,8,0));
    PASP.addToLayout(&zoomout, new RelativeAlignment(&distancePASP, 20,292,0));
    
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

    active_windows.insert(&navigation_bar);
    navigation_bar.addToLayout(&main_button, new RelativeAlignment(nullptr, 580, 15, 0));
    navigation_bar.addToLayout(&override_button, new ConsecutiveAlignment(&main_button,DOWN,0));
    navigation_bar.addToLayout(&dataview_button, new ConsecutiveAlignment(&override_button,DOWN,0));
    navigation_bar.addToLayout(&special_button, new ConsecutiveAlignment(&dataview_button,DOWN,0));
    navigation_bar.addToLayout(&config_button, new ConsecutiveAlignment(&special_button,DOWN,0));
    extern bool showSpeeds;
    csg.setPressedAction([]() {showSpeeds = !showSpeeds;});
}
