#include "../graphics/component.h"
#include "../monitor.h"
#include "../tcp/server.h"
#include "../sound/sound.h"
void dispAcks();
Component c234(37*3, 50, nullptr);
Component c2(37, 50, nullptr);
Component c3(37, 50, nullptr);
Component c4(37, 50, nullptr);
Component c1(58, 50, dispAcks);
Component c5(37, 50, nullptr);
Component c6(37, 50, nullptr);
bool prevAck = false;
int prevlevel = 0;
void dispAcks()
{
    if (modeAck == prevAck && prevlevel == levelAck) return;
    if (modeAck || levelAck == 2) playSinfo();
    prevAck = modeAck;
    prevlevel = levelAck;
    c1.clear();
    if(modeAck)
    {
        string path = "symbols/Mode/MO_";
        int num;
        switch(ackMode)
        {
            case SH:
                num = 2;
                break;
            case TR:
                num = 5;
                break;
            case OS:
                num = 8;
                break;
            case SR:
                num = 10;
                break;
            case RV:
                num = 15;
                break;
            case UN:
                num = 17;
                break;
            case NS:
                num = 20;
                break;
            case LS:
                num = 22;
                break;
        }
        if(num<10) path+="0";
        path+=to_string(num);
        path+=".bmp";
        c1.setAck([](){write_command("modeAcked","");});
        c1.addImage(path.c_str());
    }
    else if(levelAck>0)
    {
        string path = "symbols/Level/LE_";
        int num = 0;
        switch(ackLevel)
        {
            case Level::N0:
                num=1;
                break;
            case Level::NTC:
                num=2;
                break;
            case Level::N1:
                num=3;
                break;
            case Level::N2:
                num=4;
                break;
            case Level::N3:
                num=5;
                break;
        }
        num = 4 + 2*num;
        if(levelAck == 2) num++;
        if(num<10) path+="0";
        path+=to_string(num);
        //If NTC is LZB/PZB, path+="a";
        path+=".bmp";
        c1.addImage(path.c_str());
        if(levelAck == 2) c1.setAck([](){write_command("levelAcked","");});
        else c1.setAck(nullptr);
    }
    else c1.setAck(nullptr);
}