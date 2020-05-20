#pragma once
#include "../optional.h"
#include "../Position/distance.h"
extern bool overrideProcedure;
extern optional<distance> formerEoA;
void start_override();
void update_override();