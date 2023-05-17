/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _SOUND_H
#define _SOUND_H
#include <string>
#include "../../EVC/Packets/STM/46.h"
struct sdlsounddata
{
    uint8_t *wavBuffer;
    uint32_t wavLength;
    //int64_t duration;
    int freeAudio;
    ~sdlsounddata();
};
sdlsounddata *loadSound(std::string file);
sdlsounddata* loadSound(STMSoundDefinition &def);
void play(sdlsounddata *d, bool loop=false);
void stopSound(sdlsounddata *d);
void playSinfo();
void playTooFast();
void playClick();
void playSwarning();
void stopSwarning();
void start_sound();
#endif