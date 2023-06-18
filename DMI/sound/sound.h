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
#include "../platform/platform.h"
#include "../../EVC/Packets/STM/46.h"
class StmSound
{
    std::unique_ptr<UiPlatform::SoundData> data;
    std::unique_ptr<UiPlatform::SoundSource> source;

public:
    StmSound(std::unique_ptr<UiPlatform::SoundData> &&data);
    void play(bool loop);
    void stop();
};
std::unique_ptr<StmSound> loadStmSound(const std::string &name);
std::unique_ptr<StmSound> loadStmSound(const STMSoundDefinition &def);

void loadBeeps();
void playSinfo();
void playTooFast();
void playClick();
void playSwarning();
void stopSwarning();
#endif