#include <SDL2/SDL.h>
#include "sound.h"
#include <map>
#include <string>
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
void start_sound()
{
    SDL_LoadWAV("sound/S2_warning.wav", &swarn.wavSpec, &swarn.wavBuffer, &swarn.wavLength);
    SDL_LoadWAV("sound/S_info.wav", &sinfo.wavSpec, &sinfo.wavBuffer, &sinfo.wavLength);
    SDL_LoadWAV("sound/click.wav", &click.wavSpec, &click.wavBuffer, &click.wavLength);
    deviceId = SDL_OpenAudioDevice(NULL, 0, &sinfo.wavSpec, NULL, 0);
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