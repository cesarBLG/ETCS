cmake_minimum_required(VERSION 3.4.0)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR wasm32)
set(triple wasm32-wasi)

if(CMAKE_HOST_WIN32)
	set(WASI_HOST_EXE_SUFFIX ".exe")
else()
	set(WASI_HOST_EXE_SUFFIX "")
endif()

set(CMAKE_C_COMPILER ${WASI_SDK_PREFIX}/bin/clang${WASI_HOST_EXE_SUFFIX})
set(CMAKE_CXX_COMPILER ${WASI_SDK_PREFIX}/bin/clang++${WASI_HOST_EXE_SUFFIX})
set(CMAKE_ASM_COMPILER ${WASI_SDK_PREFIX}/bin/clang${WASI_HOST_EXE_SUFFIX})
set(CMAKE_AR ${WASI_SDK_PREFIX}/bin/llvm-ar${WASI_HOST_EXE_SUFFIX})
set(CMAKE_RANLIB ${WASI_SDK_PREFIX}/bin/llvm-ranlib${WASI_HOST_EXE_SUFFIX})
set(CMAKE_C_COMPILER_TARGET ${triple})
set(CMAKE_CXX_COMPILER_TARGET ${triple})
set(CMAKE_ASM_COMPILER_TARGET ${triple})

if(NOT CMAKE_HOST_WIN32)
	set(CMAKE_C_FLAGS "--sysroot=/usr/share/wasi-sysroot")
	set(CMAKE_CXX_FLAGS "--sysroot=/usr/share/wasi-sysroot")
	set(CMAKE_LD_FLAGS "--sysroot=/usr/share/wasi-sysroot")
endif()

# Don't look in the sysroot for executables to run during the build
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# Only look in the sysroot (not in the host paths) for the rest
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
