 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "console_tools.h"
#ifdef __ANDROID__
std::string android_external_storage_dir;
std::string get_files_dir(FileType type)
{
	return android_external_storage_dir;
}
#elif defined(_WIN32)
#include <string>
#include <windows.h>
#include <filesystem>
std::string getexepath()
{
  char result[ MAX_PATH ];
  return std::string( result, GetModuleFileName( NULL, result, MAX_PATH ) );
}
std::string get_files_dir(FileType type)
{
	return std::filesystem::path(getexepath()).remove_filename().string();
}
#else
#include <string>
#include <limits.h>
#include <filesystem>
#ifdef __APPLE__
#include <mach-o/dyld.h>
std::string getexepath()
{
	char result[ PATH_MAX ];
  	uint32_t count = PATH_MAX;
  	if(!_NSGetExecutablePath(result, &count))
		return std::string(result);
  	return "";
}
#else
#include <unistd.h>
std::string getexepath()
{
  char result[ PATH_MAX ];
  ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
  return std::string( result, (count > 0) ? count : 0 );
}
#endif
std::string get_files_dir(FileType type)
{
	switch (type)
	{
		case ETCS_ASSET_FILE:
			{
				auto exepath = std::filesystem::path(getexepath()).remove_filename();
				if (exepath.parent_path().filename() != "bin")
					return "";
				if (exepath == "/bin/")
					return "/usr/share/ETCS/";
				return exepath / "../share/ETCS/";
			}
		case ETCS_CONFIG_FILE:
			{
				auto exepath = std::filesystem::path(getexepath()).remove_filename();
				if (exepath.parent_path().filename() != "bin")
					return "";
				if (exepath.parent_path().parent_path().filename() == "usr")
					return exepath / "../../etc/ETCS/";
				return exepath / "../etc/ETCS/";
			}
		case ETCS_STORAGE_FILE:
			{
				const char* wd  = getenv("OWD");
				if (wd)
					return std::string(wd)+"/";
				return "";
			}
		default:
			return "";
	}
}
#endif

#ifdef _WIN32
#include <windows.h>
#include <imagehlp.h>
#include <errhandlingapi.h>
#include <psapi.h>
LONG WINAPI windows_exception_handler(EXCEPTION_POINTERS * ExceptionInfo)
{
    CONTEXT &context = *(ExceptionInfo->ContextRecord);
    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();
    
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

    return EXCEPTION_EXECUTE_HANDLER;
}
void setup_crash_handler()
{
    SetUnhandledExceptionFilter(windows_exception_handler);
}
#elif defined(__unix__) && defined(__GLIBC__)
#include <signal.h>
#include <execinfo.h>
#include <cxxabi.h>
#include <unistd.h>
int addr2line(char const * const program_name, void const * const addr)
{
    char addr2line_cmd[512] = {0};

    sprintf(addr2line_cmd,"addr2line -f -p -e %.256s %p", program_name, addr); 

    return system(addr2line_cmd);
}
void crash_handler(int sig)
{
    void *bt[20];
    size_t size = backtrace(bt, 20);
    time_t now = time(nullptr);
    FILE *file = fopen("error.log", "w+");
    fprintf(file, "ETCS received signal %d, date: %s\n", sig, ctime(&now));
    backtrace_symbols_fd(bt, size, fileno(file));
    fclose(file);
    
    signal(sig, SIG_DFL);
    return;
}
void setup_crash_handler()
{
    signal(SIGSEGV, crash_handler);
    signal(SIGABRT, crash_handler);
}
#else
void setup_crash_handler()
{

}
#endif