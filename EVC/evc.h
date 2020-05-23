#pragma once
#ifdef DLL
extern "C" {
__declspec( dllexport ) void start();
__declspec( dllexport ) void update();
}
#else
extern "C" {
void start();
void update();
}
#endif