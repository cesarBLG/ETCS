#include "time_hour.h"
#include "../time.h"

Component time_hour(63,50,timeHour);
void timeHour()
{
    char time[9];
    snprintf(time,9,"%02d:%02d:%02d",getHour(),getMinute(),getSecond());
    time_hour.setText(time,10,White);
}