#pragma once
#include <set>
#define VERSION_X(ver) ((ver)>>4)
#define VERSION_Y(ver) ((ver)&15)
extern int operated_version;
extern std::set<int> supported_versions;
bool is_version_supported(int version);
void operate_version(int version, bool rbc);