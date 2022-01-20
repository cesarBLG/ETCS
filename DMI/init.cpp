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
#include <thread>
#include "monitor.h"
#include "graphics/drawing.h"
#include "control/control.h"
#include "tcp/server.h"
bool running = true;
void quit()
{
    //std::unique_lock<std::mutex> lck(window_mtx);
    running = false;
    //window_cv.notify_one();
    printf("quit\n");
}
#ifdef __ANDROID__
#elif defined(_WIN32)
#include <windows.h>
#include <imagehlp.h>
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
            fprintf(file, "[%i] %s\n", i, symbol->Name);
        } else {
            fprintf(file, "[%i] %p\n", i, (void*)stackframe.AddrPC.Offset);
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
void sighandler(int sig)
{
    quit();
}
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
std::string filesDir;
extern "C" void Java_com_etcs_dmi_DMI_DMIstop(JNIEnv *env, jobject thiz)
{
    running = false;
}
#endif
int main(int argc, char** argv)
{
#ifdef __ANDROID__
    filesDir = SDL_AndroidGetExternalStoragePath();
#else
#ifdef __unix__
    signal(SIGINT, sighandler);
    signal(SIGSEGV, crash_handler);
    signal(SIGABRT, crash_handler);
#endif
#endif
#ifdef _WIN32
    SetUnhandledExceptionFilter(windows_exception_handler);
#endif
    setSpeeds(0, 0, 0, 0, 0, 0);
    setMonitor(CSM);
    setSupervision(NoS);
    //std::thread video(init_video);
    std::thread tcp(startSocket);
    startWindows();
    init_video();
    //manage_windows();
    tcp.join();
    //video.join();
    return 0;
}
