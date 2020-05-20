#include "time_hour.h"
#include "../time.h"
int lastsec = -1;
Component time_hour(63,50,timeHour);
void timeHour()
{
    if (lastsec == getSecond()) return;
    lastsec = getSecond();
    time_hour.clear();
    char time[9];
    snprintf(time,9,"%02d:%02d:%02d",getHour(),getMinute(),getSecond());
    time_hour.addText(time,0,0,10,White);
}