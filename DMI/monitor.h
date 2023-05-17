/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _MONITOR_H
#define _MONITOR_H
#include <string>
#include "../EVC/Supervision/common.h"
extern Level level;
extern int nid_ntc;
extern bool level_valid;
extern int levelAck;
extern Level ackLevel;
extern int ackNTC;
extern Mode mode;
extern bool modeAck;
extern Mode ackMode;
extern int radioStatus;
extern MonitoringStatus monitoring;
extern SupervisionStatus supervision;
extern float Vtarget;
extern float Vperm;
extern float Vsbi;
extern float Vebi;
extern float Vrelease;
extern float Vest;
extern float Vset;
extern float Dtarg;
extern float TTI;
extern float TTP;
extern bool SB;
extern bool EB;
extern bool brakeAck;
extern int trn;
extern bool trn_valid;
extern std::string driverid;
extern bool driverid_valid;
extern bool ovEOA;
extern bool train_data_valid;
extern int L_train;
extern bool ovEOA;
void update();
void setSpeeds(float vtarg, float vperm, float vsbi, float vrelease, float vest, float dist);
void setMonitor(MonitoringStatus status);
void setSupervision(SupervisionStatus status);
#endif