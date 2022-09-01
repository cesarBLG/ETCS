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
struct sdlsounddata
{
    Uint8 *wavBuffer;
    SDL_AudioSpec wavSpec;
    Uint32 wavLength;
};
sdlsounddata sinfo;
sdlsounddata swarn;
sdlsounddata stoofast;
sdlsounddata click; 
SDL_AudioDeviceID deviceId;
time_t last_sinfo;
void start_sound()
{
#ifdef __ANDROID__
    extern std::string filesDir;
    SDL_LoadWAV((filesDir+"/sound/S2_warning.wav").c_str(), &swarn.wavSpec, &swarn.wavBuffer, &swarn.wavLength);
    SDL_LoadWAV((filesDir+"/sound/S_info.wav").c_str(), &sinfo.wavSpec, &sinfo.wavBuffer, &sinfo.wavLength);
    SDL_LoadWAV((filesDir+"/sound/S1_toofast.wav").c_str(), &stoofast.wavSpec, &stoofast.wavBuffer, &stoofast.wavLength);
    SDL_LoadWAV((filesDir+"/sound/click.wav").c_str(), &click.wavSpec, &click.wavBuffer, &click.wavLength);
#else
    SDL_LoadWAV("sound/S2_warning.wav", &swarn.wavSpec, &swarn.wavBuffer, &swarn.wavLength);
    SDL_LoadWAV("sound/S_info.wav", &sinfo.wavSpec, &sinfo.wavBuffer, &sinfo.wavLength);
    SDL_LoadWAV("sound/S1_toofast.wav", &stoofast.wavSpec, &stoofast.wavBuffer, &stoofast.wavLength);
    SDL_LoadWAV("sound/click.wav", &click.wavSpec, &click.wavBuffer, &click.wavLength);
#endif
    deviceId = SDL_OpenAudioDevice(NULL, 0, &sinfo.wavSpec, NULL, 0);
    last_sinfo = time(nullptr)-1;
}
Uint32 warnLength;
Uint8 *warnBuffer = nullptr;
Uint32 refill(Uint32 interval, void *param)
{
    if(warnBuffer != NULL && SDL_GetQueuedAudioSize(deviceId) < 3*warnLength) SDL_QueueAudio(deviceId, warnBuffer, warnLength);
    if(warnBuffer == NULL) return 0;
    return interval;
}
void stop()
{
    SDL_ClearQueuedAudio(deviceId);
    warnBuffer = nullptr;
    SDL_PauseAudioDevice(deviceId, 1);
}
void play(sdlsounddata d, bool loop=false)
{
    if(warnBuffer != nullptr) stop();
    
    SDL_ClearQueuedAudio(deviceId);
    int success = SDL_QueueAudio(deviceId, d.wavBuffer, d.wavLength);
    SDL_PauseAudioDevice(deviceId, 0);
    if(loop)
    {
        warnBuffer = d.wavBuffer;
        warnLength = d.wavLength;
        SDL_AddTimer(50, refill, NULL);
    }
}
void playSwarning()
{
    if (warnBuffer == nullptr)
        play(swarn, true);
}
void stopSwarning()
{
    stop();
}
void playSinfo()
{
    if (time(nullptr)-last_sinfo<1) return;
    last_sinfo = time(nullptr);
    play(sinfo);
}
void playTooFast()
{
    play(stoofast);
}
void playClick()
{
    play(click);
}
