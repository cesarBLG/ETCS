#include "../graphics/component.h"
#include <string>
#include "../monitor.h"
using namespace std;
extern Component csg;
int track_conditions[] = {0,0,0};
void dispTc1();
void dispTc2();
void dispTc3();
Component b3(36, 36, dispTc1);
Component b4(36, 36, dispTc2);
Component b5(36, 36, dispTc3);
void dispTc(int num, Component &b)
{
    /*if(num == 1 && level == NTC)
    {
        b.setText(to_string((int)Vtarget).c_str(), 15, Red);
        return;
    }*/
    if(track_conditions[num]==0) return;
    string path = "symbols/Track Conditions/TC_";
    if(track_conditions[num]<10) path+= "0";
    path+= to_string(track_conditions[num]);
    path+= ".bmp";
    b.drawImage(path.c_str());
}
void dispTc1(){dispTc(0, b3);}
void dispTc2(){dispTc(1, b4);}
void dispTc3(){dispTc(2, b5);}
void addTc(int newtc)
{
    for(int i=0; i<3; i++)
    {
        if(track_conditions[i]==0)
        {
            track_conditions[i] = newtc;
            break;
        }
    }
}