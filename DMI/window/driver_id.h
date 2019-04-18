#ifndef _DRIVER_ID_H
#define _DRIVER_ID_H
#include "data_entry.h"
class driver_window : public input_window
{
    TextButton TRN;
    IconButton settings;
    public:
    driver_window();
    void validate(string dat) override;
};
#endif