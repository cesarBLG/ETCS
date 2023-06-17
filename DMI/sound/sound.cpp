/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "sound.h"
#include <map>
#include <string>
#include <ctime>

StmSound::StmSound(std::unique_ptr<Platform::SoundData> &&in) : data(std::move(in)) {

}

void StmSound::play(bool loop) {
    source = platform->play_sound(*data, loop);
}

void StmSound::stop() {
    source = nullptr;
}

std::unique_ptr<StmSound> loadStmSound(const std::string &path)
{
    size_t slash = path.find_last_of('/');
    std::string filename = path.substr(slash != -1 ? slash + 1 : 0, -1);
    size_t dot = path.find_last_of('.');
    std::string basename = filename.substr(0, dot);
    return std::make_unique<StmSound>(platform->load_sound(basename));
}

std::unique_ptr<StmSound> loadStmSound(const STMSoundDefinition &def)
{
    std::vector<std::pair<int, int>> melody;
    for (auto &part : def.parts)
        melody.push_back(std::make_pair(part.M_FREQ.get_value() * 32, part.T_SOUND.get_value() * 100));
    return std::make_unique<StmSound>(platform->load_sound(melody));
}

std::unique_ptr<Platform::SoundData> sinfo;
std::unique_ptr<Platform::SoundData> swarn;
std::unique_ptr<Platform::SoundData> stoofast;
std::unique_ptr<Platform::SoundData> click; 
time_t last_sinfo;
std::unique_ptr<Platform::SoundSource> swarn_source;

void loadBeeps()
{
    sinfo = platform->load_sound("S_info");
    swarn = platform->load_sound("S2_warning");
    stoofast = platform->load_sound("S1_toofast");
    click = platform->load_sound("click");
}

void playSwarning()
{
    if (swarn_source)
        return;
    swarn_source = platform->play_sound(*swarn, true);
}

void stopSwarning()
{
    swarn_source = nullptr;
}

void playSinfo()
{
    if (time(nullptr)-last_sinfo<1) return;
    last_sinfo = time(nullptr);
    platform->play_sound(*sinfo, false)->detach();
}

void playTooFast()
{
    platform->play_sound(*stoofast, false)->detach();
}

void playClick()
{
    platform->play_sound(*click, false)->detach();
}
