#pragma once
#include "../Position/distance.h"
#include "../Time/clock.h"
#include "messages.h"
void start_logging();
void log_message(std::shared_ptr<ETCS_message> msg, distance &dist, int64_t time);