#include <ctime>
#include "time.h"
using namespace std;
tm getTime()
{
    time_t t = time(nullptr);
    return *localtime(&t);
}
int getHour()
{
    return getTime().tm_hour;
}
int getMinute()
{
    return getTime().tm_min;
}