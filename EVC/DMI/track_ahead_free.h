#pragma once
#include "../Position/distance.h"
extern bool start_display_taf;
extern bool stop_display_taf;
void request_track_ahead_free(distance start, double length);
void track_ahead_free_granted();
void update_track_ahead_free_request();