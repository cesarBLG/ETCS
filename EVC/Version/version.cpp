/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "version.h"
#include "../Supervision/supervision.h"
#include "../Euroradio/session.h"
#include "../TrainSubsystems/cold_movement.h"
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
            json ver(operated_version);
            save_cold_data("operated_version", ver);
            return;
        }
    }
}
void load_version()
{
    json ver = load_cold_data("operated_version");
    if (ver.is_null()) operated_version = *--supported_versions.end();
    else operated_version = ver;
}