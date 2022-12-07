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
#include <SDL.h>
#include "sound.h"
#include <map>
#include <string>
#include <ctime>
using namespace std;
sdlsounddata sinfo;
sdlsounddata swarn;
sdlsounddata stoofast;
sdlsounddata click; 
SDL_AudioSpec deviceSpec;
SDL_AudioDeviceID deviceId;
time_t last_sinfo;
void start_sound()
{
    SDL_AudioSpec spec;
    std::string soundpath;
#ifdef __ANDROID__
    extern std::string filesDir;
    soundpath = filesDir+"/sound/";
#else
    soundpath = "sound/";
#endif
    SDL_LoadWAV((soundpath+"S2_warning.wav").c_str(), &spec, &swarn.wavBuffer, &swarn.wavLength);
    SDL_LoadWAV((soundpath+"S_info.wav").c_str(), &spec, &sinfo.wavBuffer, &sinfo.wavLength);
    SDL_LoadWAV((soundpath+"S1_toofast.wav").c_str(), &spec, &stoofast.wavBuffer, &stoofast.wavLength);
    SDL_LoadWAV((soundpath+"click.wav").c_str(), &spec, &click.wavBuffer, &click.wavLength);

    deviceId = SDL_OpenAudioDevice(NULL, 0, &spec, &deviceSpec, 0);
    last_sinfo = time(nullptr)-1;
}
sdlsounddata* loadSound(std::string file)
{
#ifdef __ANDROID__
    extern std::string filesDir;
    file = filesDir+"/sound/"+file;
#else
    file = "sound/"+file;
#endif
    auto *snd = new sdlsounddata();
    snd->freeAudio = 1;
    SDL_AudioSpec spec;
    SDL_LoadWAV(file.c_str(), &spec, &snd->wavBuffer, &snd->wavLength);
    //snd->duration = snd->wavLength * 500 / spec.freq;
    return snd;
}
sdlsounddata* loadSound(STMSoundDefinition &def)
{
    std::vector<Uint16> buff;
    //int64_t duration = 0;
    for (auto &part : def.parts)
    {
        float freq = (float)part.M_FREQ.get_value();
        int nsteps = deviceSpec.freq * part.T_SOUND.get_value();
        float factor = 2*M_PI*freq/deviceSpec.freq;
        for (int i=0; i<nsteps; i++)
        {
            buff.push_back(freq == 0 ? 0 : 28000*sinf(i * factor));
        }
        //duration += part.T_SOUND.rawdata * 100;
    }
    auto *snd = new sdlsounddata();
    snd->wavLength = buff.size() * 2;
    snd->wavBuffer = new Uint8[snd->wavLength];
    snd->freeAudio = 2;
    //snd->duration = duration;
    memcpy(snd->wavBuffer, &buff[0], snd->wavLength);
    return snd;
}
sdlsounddata::~sdlsounddata()
{
    if (freeAudio == 1) SDL_FreeWAV(wavBuffer);
    else if (freeAudio == 2) delete wavBuffer;
}
sdlsounddata *continuousSound;
Uint32 refill(Uint32 interval, void *param)
{
    if(continuousSound == nullptr) return 0;
    if(SDL_GetQueuedAudioSize(deviceId) < 3*continuousSound->wavLength) SDL_QueueAudio(deviceId, continuousSound->wavBuffer, continuousSound->wavLength);
    return interval;
}
void stopSound(sdlsounddata *d)
{
    if (d != nullptr && d != continuousSound) return;
    SDL_ClearQueuedAudio(deviceId);
    continuousSound = nullptr;
    SDL_PauseAudioDevice(deviceId, 1);
}
void play(sdlsounddata *d, bool loop)
{
    if(continuousSound != nullptr) stopSound(continuousSound);
    
    SDL_ClearQueuedAudio(deviceId);
    int success = SDL_QueueAudio(deviceId, d->wavBuffer, d->wavLength);
    SDL_PauseAudioDevice(deviceId, 0);
    if(loop)
    {
        continuousSound = d;
        SDL_AddTimer(50, refill, NULL);
    }
}
void playSwarning()
{
    if (continuousSound == nullptr)
        play(&swarn, true);
}
void stopSwarning()
{
    stopSound(&swarn);
}
void playSinfo()
{
    if (time(nullptr)-last_sinfo<1) return;
    last_sinfo = time(nullptr);
    play(&sinfo);
}
void playTooFast()
{
    play(&stoofast);
}
void playClick()
{
    play(&click);
}
