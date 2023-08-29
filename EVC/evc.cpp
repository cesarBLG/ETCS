/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "TrainSubsystems/cold_movement.h"
#include "DMI/dmi.h"
#include "DMI/text_message.h"
#include "DMI/windows.h"
#include "DMI/track_ahead_free.h"
#include "Packets/messages.h"
#include "Packets/logging.h"
#include "Supervision/speed_profile.h"
#include "Supervision/targets.h"
#include "Supervision/supervision.h"
#include "Position/distance.h"
#include "Position/geographical.h"
#include "Supervision/conversion_model.h"
#include "OR_interface/interface.h"
#include "MA/movement_authority.h"
#include "Procedures/procedures.h"
#include "NationalFN/nationalfn.h"
#include "TrackConditions/track_condition.h"
#include "TrainSubsystems/subsystems.h"
#include "LX/level_crossing.h"
#include "STM/stm.h"
#include "Euroradio/terminal.h"
#include "platform_runtime.h"

#if defined(_WIN32)
#include <windows.h>
#include <imagehlp.h>
#include <errhandlingapi.h>
#include <psapi.h>
void print_stack(CONTEXT &context)
{
    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();

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
        
        if (!result) { 
            if (i == 0) fprintf(file, "Failed to get stack trace: error %d\n", GetLastError()); 
            break;
        }
        
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
}
void crash_handler(int sig)
{
    CONTEXT context;
    memset(&context, 0, sizeof(CONTEXT));
    context.ContextFlags = CONTEXT_FULL;
    RtlCaptureContext(&context);
    print_stack(context);
}
LONG WINAPI windows_exception_handler(EXCEPTION_POINTERS * ExceptionInfo)
{
    print_stack(*(ExceptionInfo->ContextRecord));
    return EXCEPTION_EXECUTE_HANDLER;
}
#elif defined(__unix__)
#include <signal.h>
#include <execinfo.h>
#include <cxxabi.h>
#include <unistd.h>
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

void update();

void on_platform_ready()
{
#ifndef __ANDROID__
#if __unix__
    signal(SIGSEGV, crash_handler);
    signal(SIGABRT, crash_handler);
#elif defined(_WIN32)
    SetUnhandledExceptionFilter(windows_exception_handler);
#endif
#endif

    platform->debug_print("Starting European Train Control System...");
    platform->on_quit_request().then([](){
        json odo(odometer_value);
        save_cold_data("odometer_value", odo);
        platform->quit();
    }).detach();

    start_dmi();
    start_or_iface();
    start_logging();
    initialize_mode_transitions();
    setup_stm_control();
    set_message_filters();
    initialize_national_functions();
    platform->delay(500).then(update).detach();
}
void update()
{
    update_or_iface();
    update_odometer();
    update_geographical_position();
    update_track_comm();
    update_national_values();
    update_procedures();
    update_stm_control();
    update_supervision();
    update_lx();
    update_track_conditions();
    update_messages();
    update_national_functions();
    update_train_subsystems();
    update_dmi_windows();
    update_track_ahead_free_request();
    platform->delay(50).then(update).detach();
}
