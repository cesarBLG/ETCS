#ifndef _TRAIN_DATA_WINDOW_H
#define _TRAIN_DATA_WINDOW_H
#include "data_entry.h"
class train_data_window : public input_window
{
    public:
    train_data_window();
    void validate(string dat, int field)
    {

    }
    void sendInformation() override;
};
#endif