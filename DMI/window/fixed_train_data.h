#ifndef _FIXED_TRAIN_DATA_WINDOW_H
#define _FIXED_TRAIN_DATA_WINDOW_H
#include "data_entry.h"
class fixed_train_data_window : public input_window
{
    public:
    TextButton SelectType;
    fixed_train_data_window();
    void validate(string dat, int field)
    {

    }
    void sendInformation() override;
    void setLayout() override;
};
class fixed_train_data_input : public input_data
{
    public:
    fixed_train_data_input();
    void validate() override;
};
#endif