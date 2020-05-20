#include "gps_pos.h"
#include "../time.h"
Component gps_pos(120,50,display_gps);
static bool show_pos = false;
static bool f = false;
int prevkm=-1;
int prevm=-1;
double pk=-1;
void display_gps()
{
    if (show_pos) {
        if (pk<0)
        {
            show_pos = false;
            gps_pos.clear();
            gps_pos.addImage("symbols/Driver Request/DR_03.bmp");
            gps_pos.setBackgroundColor(DarkBlue);
            return;
        }
        int m = ((int)pk)%1000;
        int km = (int)(pk/1000);
        string tm = to_string(m/100)+to_string((m/10)%10)+to_string(m%10);
        if (prevkm != km || prevm != m)
        {
            gps_pos.clear();
            gps_pos.addText(to_string(km), 62, 0, 12, Black, RIGHT);
            gps_pos.addText(tm, 62, 0, 10, Black, LEFT);
            prevm = m;
            prevkm = km;
            gps_pos.paint();
        }
    }
    if(f) return;
    f = true;
    gps_pos.addImage("symbols/Driver Request/DR_03.bmp");
    gps_pos.setPressedAction([]() {
        show_pos = !show_pos && pk >= 0;
        gps_pos.clear();
        if (!show_pos)
        {
            gps_pos.addImage("symbols/Driver Request/DR_03.bmp");
            gps_pos.setBackgroundColor(DarkBlue);
        }
        else
        {
            prevkm = prevm = -1;
            gps_pos.setBackgroundColor(Grey);
        }
    });
    gps_pos.paint();
}
