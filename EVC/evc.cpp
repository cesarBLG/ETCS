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
#include <cstdio>
#include <mutex>
#include <condition_variable>
#include "DMI/dmi.h"
#include "DMI/text_message.h"
#include <iostream>
#include <chrono>
#include "Packets/messages.h"
#include "Supervision/speed_profile.h"
#include "Supervision/targets.h"
#include "Supervision/supervision.h"
#include "Supervision/national_values.h"
#include "Position/distance.h"
#include "Position/geographical.h"
#include "Supervision/conversion_model.h"
#include "OR_interface/interface.h"
#include "MA/movement_authority.h"
#include "Procedures/procedures.h"
#include "NationalFN/nationalfn.h"
#include "TrackConditions/track_condition.h"
#include "TrainSubsystems/subsystems.h"

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
#else
#include <execinfo.h>
#include <cxxabi.h>
#include <unistd.h>
#include <signal.h>
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


std::mutex loop_mtx;
std::condition_variable loop_notifier;
void loop();
void start();
bool run;
int main()
{
    run = true;
#ifdef __ANDROID__
#else
#ifdef __unix__
    signal(SIGSEGV, crash_handler);
    signal(SIGABRT, crash_handler);
#endif
#endif
#ifdef _WIN32
    SetUnhandledExceptionFilter(windows_exception_handler);
#endif
    std::printf("Starting European Train Control System...\n");
    start();
    loop();
    return 0;
}

#ifdef __ANDROID__
#include <jni.h>
extern "C" void Java_com_etcs_dmi_EVC_evcMain(JNIEnv *env, jobject thiz)
{
    main();
}
extern "C" void Java_com_etcs_dmi_EVC_evcStop(JNIEnv *env, jobject thiz)
{
    run = false;
}
#endif
bool started=false;
void start()
{
    start_dmi();
    start_or_iface();
    setup_national_values();
    initialize_mode_transitions();
    ETCS_packet::initialize();
    set_message_filters();
    initialize_national_functions();
    started = true;
}
void update()
{
    std::unique_lock<std::mutex> lck(loop_mtx);
    update_odometer();
    update_geographical_position();
    check_eurobalise_passed();
    update_national_values();
    update_procedures();
    update_supervision();
    update_track_conditions();
    update_messages();
    update_national_functions();
    update_train_subsystems();
}
void loop()
{
    while(run)
    {
        auto prev = std::chrono::system_clock::now();
        update();
        std::chrono::duration<double> diff = std::chrono::system_clock::now() - prev;
        int d = std::chrono::duration_cast<std::chrono::duration<int, std::micro>>(diff).count();
        //if (d>1000) std::cout<<d<<std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }
}
