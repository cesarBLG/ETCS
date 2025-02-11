/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "monitor.h"
#include "distance/distance.h"
#include "sound/sound.h"
MonitoringStatus monitoring;
SupervisionStatus supervision;
Level level = Level::Unknown;
int nid_ntc;
int radioStatus;
bool level_valid;
int levelAck;
Level ackLevel;
int ackNTC;
Mode mode;
bool modeAck;
Mode ackMode;
float Vtarget;
float Vperm;
float Vsbi;
float Vebi;
float Vrelease;
float Vest;
float Vset=-1;
float Dtarg;
float TTI = 20;
float TTP = 20;
bool SB;
bool EB;
bool brakeAck;
int trn;
bool trn_valid;
std::string driverid;
bool driverid_valid;
bool ovEOA;
bool train_data_valid;
int L_train;
bool slippery_rail;
bool bot_driver;

void updateSupervision();
void update()
{
    /*if(monitoring == CSM)
    {
        if(prevVperm >= 0 && prevVperm<Vperm) playSinfo();
        prevVperm = Vperm;
    }
    else prevVperm = -1;*/
    //updateSupervision();
}
void setSpeeds(float vtarg, float vperm, float vsbi, float vrelease, float vest, float dist)
{
    Vtarget = vtarg;
    Vperm = vperm;
    Vsbi = vsbi;
    Vrelease = vrelease;
    Vest = vest;
    Dtarg = dist;
}
void setMonitor(MonitoringStatus status)
{
    if(monitoring == CSM && (status == TSM || status == RSM)) playSinfo();
    monitoring = status;
}
void setSupervision(SupervisionStatus status)
{
    if(monitoring == TSM && supervision == IndS && status == OvS) playTooFast();
    if(status == WaS) playSwarning();
    else if(supervision == WaS) stopSwarning();
    supervision = status;
}