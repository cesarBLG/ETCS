#include <SDL2/SDL.h>
#include "sound.h"
#include <stdint.h>
bool open = false;
SDL_AudioDeviceID deviceId;
Uint32 warnLength;
Uint8 *warnBuffer = nullptr;
void play(const char* name)
{
    SDL_AudioSpec wavSpec;
    Uint32 wavLength;
    Uint8 *wavBuffer;
    
    SDL_LoadWAV(name, &wavSpec, &wavBuffer, &wavLength);
    if(warnBuffer != nullptr) stopSwarning();
    if(open) SDL_CloseAudioDevice(deviceId);
    deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);
    int success = SDL_QueueAudio(deviceId, wavBuffer, wavLength);
    SDL_PauseAudioDevice(deviceId, 0);
    SDL_FreeWAV(wavBuffer);
    open = true;
}
void checkSound()
{
    if(!open) return;
    if(warnBuffer != nullptr && SDL_GetQueuedAudioSize(deviceId) < 100000) SDL_QueueAudio(deviceId, warnBuffer, warnLength);
    if(SDL_GetQueuedAudioSize(deviceId) == 0)
    {
        open = false;
        SDL_CloseAudioDevice(deviceId);
    }
}
void playSwarning()
{
    if(warnBuffer != nullptr) return;
    SDL_AudioSpec wavSpec;
    
    SDL_LoadWAV("sound/S2_warning.wav", &wavSpec, &warnBuffer, &warnLength);
    if(open) SDL_CloseAudioDevice(deviceId);
    deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);
    int success = SDL_QueueAudio(deviceId, warnBuffer, warnLength);
    SDL_PauseAudioDevice(deviceId, 0);
    open = true;
}
void stopSwarning()
{
    SDL_FreeWAV(warnBuffer);
    warnBuffer = nullptr;
    if(open) SDL_CloseAudioDevice(deviceId);
    open = false;
}
void playSinfo()
{
    play("sound/S_info.wav");
}
void playTooFast()
{
    play("sound/S1_toofast.wav");
}
void playClick()
{
    play("sound/click.wav");
}