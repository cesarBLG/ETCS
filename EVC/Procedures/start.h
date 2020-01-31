#pragma once
#include "../Supervision/train_data.h"
#include "mode_transition.h"
void start_mission()
{
    if (V_train > 0 && L_TRAIN > 0 && brake_percentage > 0) {
        set_conversion_model();
        mode_conditions[8].trigger();
    }
}