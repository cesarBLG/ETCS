#include "clock.h"
int64_t get_milliseconds()
{
    return (std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::system_clock::now().time_since_epoch())).count();
}