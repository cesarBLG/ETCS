/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "monitor.h"
#include "graphics/drawing.h"
#include "tcp/server.h"
#include "control/control.h"
#include "platform.h"
#ifdef _WIN32
#include <windows.h>
#include <imagehlp.h>
#include <errhandlingapi.h>
#include <psapi.h>
LONG WINAPI windows_exception_handler(EXCEPTION_POINTERS * ExceptionInfo)
{
    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();
    
    CONTEXT context = *(ExceptionInfo->ContextRecord);
    /*memset(&context, 0, sizeof(CONTEXT));
    context.ContextFlags = CONTEXT_FULL;
    RtlCaptureContext(&context);*/
    
    SymInitialize(process, NULL, TRUE);
    
    DWORD image;
    STACKFRAME64 stackframe;
    ZeroMemory(&stackframe, sizeof(STACKFRAME64));
    
   /* #ifdef _M_IX86
    image = IMAGE_FILE_MACHINE_I386;
    stackframe.AddrPC.Offset = context.Eip;
    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrFrame.Offset = context.Ebp;
    stackframe.AddrFrame.Mode = AddrModeFlat;
    stackframe.AddrStack.Offset = context.Esp;
    stackframe.AddrStack.Mode = AddrModeFlat;
    #elif _M_X64*/
    image = IMAGE_FILE_MACHINE_AMD64;
    stackframe.AddrPC.Offset = context.Rip;
    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrFrame.Offset = context.Rbp;
    stackframe.AddrFrame.Mode = AddrModeFlat;
    stackframe.AddrStack.Offset = context.Rsp;
    stackframe.AddrStack.Mode = AddrModeFlat;
    /*#elif _M_IA64
    image = IMAGE_FILE_MACHINE_IA64;
    stackframe.AddrPC.Offset = context.StIIP;
    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrFrame.Offset = context.IntSp;
    stackframe.AddrFrame.Mode = AddrModeFlat;
    stackframe.AddrBStore.Offset = context.RsBSP;
    stackframe.AddrBStore.Mode = AddrModeFlat;
    stackframe.AddrStack.Offset = context.IntSp;
    stackframe.AddrStack.Mode = AddrModeFlat;
    #endif*/

    MODULEINFO moduleInfo;
    GetModuleInformation(process, GetModuleHandleA(NULL), &moduleInfo, sizeof(MODULEINFO));
    
    FILE *file = fopen("error.log", "w+");
    for (size_t i = 0; i < 25; i++) {
        
        BOOL result = StackWalk64(
        image, process, thread,
        &stackframe, &context, NULL, 
        SymFunctionTableAccess64, SymGetModuleBase64, NULL);
        
        if (!result) { break; }
        
        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;
        
        DWORD64 displacement = 0;
        if (SymFromAddr(process, stackframe.AddrPC.Offset, &displacement, symbol)) {
            fprintf(file, "[%i] %s", i, symbol->Name);
            IMAGEHLP_LINE64 line;
            line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
            DWORD d;
            if (SymGetLineFromAddr64(process, stackframe.AddrPC.Offset, &d, &line)) {
                fprintf(file, " %s:%i\n", line.FileName, line.LineNumber);
            } else {
                fprintf(file, "\n");
            }
        } else {
            fprintf(file, "[%i] %p\n", i, (void*)(stackframe.AddrPC.Offset-(unsigned long long)moduleInfo.lpBaseOfDll+0x140000000ULL));
        }
        
    }
    fclose(file);
    
    SymCleanup(process);
    return EXCEPTION_EXECUTE_HANDLER;
}
#elif defined(__unix__)
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <cxxabi.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
int addr2line(char const * const program_name, void const * const addr)
{
    char addr2line_cmd[512] = {0};

    sprintf(addr2line_cmd,"addr2line -f -p -e %.256s %p", program_name, addr); 

    return system(addr2line_cmd);
}
#define MAX_STACK_FRAMES 64
static void *stack_traces[MAX_STACK_FRAMES];
void crash_handler(int sig)
{
    void *bt[20];
    size_t size = backtrace(bt, 20);
    time_t now = time(nullptr);
    FILE *file = fopen("error.log", "w+");
    fprintf(file, "DMI received signal %d, date: %s\n", sig, ctime(&now));
    backtrace_symbols_fd(bt, size, fileno(file));
    fclose(file);
    
    signal(sig, SIG_DFL);
    return;
}
#endif
#ifdef __ANDROID__
#include <jni.h>
extern "C" void Java_com_etcs_dmi_DMI_DMIstop(JNIEnv *env, jobject thiz)
{
    if (platform)
        platform->quit();
}
#endif

void startWindows();
void initialize_stm_windows();

float platform_size_w = 640.0f, platform_size_h = 480.0f;

void on_platform_ready()
{
#ifndef __ANDROID__
#ifdef __unix__
    signal(SIGSEGV, crash_handler);
    signal(SIGABRT, crash_handler);
#endif
#endif
#ifdef _WIN32
    SetUnhandledExceptionFilter(windows_exception_handler);
#endif

    platform->on_quit_request().then([](){
        platform->quit();
    }).detach();

    setSpeeds(0, 0, 0, 0, 0, 0);
    setMonitor(CSM);
    setSupervision(NoS);
    startSocket();
    startWindows();
    initialize_stm_windows();
    drawing_start();
}
