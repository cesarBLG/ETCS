#ifndef _RUNNING_NUMBER_H
#define _RUNNING_NUMBER_H
#include "data_entry.h"
class trn_window : public input_window
{
    public:
    trn_window();
    void sendInformation() override;
};
class trn_input : public input_data
{
    public:
    trn_input();
    void validate() override;
};
#endif