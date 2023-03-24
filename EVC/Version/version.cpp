#include "version.h"
#include "../Supervision/supervision.h"
#include "../Euroradio/session.h"
int operated_version=33;
std::set<int> supported_versions = {33, 17};
bool is_version_supported(int version)
{
    for (int v : supported_versions) {
        if (VERSION_X(v) == VERSION_X(version))
            return true;
    }
    return false;
}
void operate_version(int version, bool rbc) {
    if (!rbc && supervising_rbc && supervising_rbc->status == session_status::Established && (level == Level::N2 || level == Level::N3))
        return;
    for (int v : supported_versions) {
        if (VERSION_X(v) == VERSION_X(version)) {
            operated_version = v;
            return;
        }
    }
}