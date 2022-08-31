/*
 * European Train Control System
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
float Vset;
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