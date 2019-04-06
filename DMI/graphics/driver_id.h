#ifndef _DRIVER_ID_H
#define _DRIVER_ID_H
#include "data_entry.h"
#include "button.h"
#include "component.h"
class driver_window : public input_window
{
    public:
    driver_window();
    void validate(string dat) override;
};
#endif