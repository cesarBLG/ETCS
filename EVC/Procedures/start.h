#pragma once
#include "../Supervision/train_data.h"
#include "mode_transition.h"
void start_mission()
{
    if (train_data_valid()) {
        if (level == Level::N0) {
            mode_to_ack = Mode::UN;
            mode_acknowledgeable = true;
            mode_acknowledged = false;
        } else {
            mode_to_ack = Mode::SR;
            mode_acknowledgeable = true;
            mode_acknowledged = false;
        }
    }
}