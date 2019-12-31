#ifndef _DRIVER_ID_H
#define _DRIVER_ID_H
#include "data_entry.h"
class driver_window : public input_window
{
    TextButton TRN;
    IconButton settings;
    public:
    driver_window();
    void setLayout() override;
    void sendInformation() override;
};
class driverid_input : public input_data
{
    public:
    driverid_input();
    void validate() override;
};
#endif