#include "time_hour.h"
#include "../time.h"

Component time_hour(63,50,timeHour);
void timeHour()
{
    time_hour.drawBox(63,50, DarkBlue);
    time_hour.setText((to_string(getHour())+":"+to_string(getMinute())+":"+to_string(getSecond())).c_str(),10,White);
}