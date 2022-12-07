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