#pragma once
#include "../Supervision/supervision.h"
#include "../Supervision/train_data.h"
void start_mission()
{
    if (V_train > 0 && L_TRAIN > 0 && brake_percentage > 0)
        mode = Mode::SR;
}