#pragma once
#include "mode_transition.h"
#include "level_transition.h"
#include "override.h"
#include "train_trip.h"
void update_procedures()
{
    update_level_status();
    update_mode_status();
    update_override();
    update_trip();
}