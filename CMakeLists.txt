cmake_minimum_required (VERSION 3.14)
project (ETCS)
set(CMAKE_CXX_STANDARD 17)
option(SIMRAIL "SimRail" OFF)
option(DEBUG_VERBOSE "Print debug messages" ON)
if (NOT ${CMAKE_SYSTEM_PROCESSOR} MATCHES "wasm.*")
    set (WASM FALSE)
else()
    set (WASM TRUE)
endif()
set(RADIO_CFM OFF)
if (NOT WASM)
    if (NOT SIMRAIL)
        set (RADIO_CFM ON)
    endif()
    option(ETCS_VENDORED "Use vendored libraries" ON)
    if (ETCS_VENDORED)
        set(SDL2TTF_VENDORED ON CACHE BOOL "Vendored TTF libs")
        add_subdirectory(libs/SDL EXCLUDE_FROM_ALL)
        add_subdirectory(libs/SDL_ttf EXCLUDE_FROM_ALL)
        add_subdirectory(libs/liborts EXCLUDE_FROM_ALL)
        if (RADIO_CFM)
            add_subdirectory(libs/c-ares EXCLUDE_FROM_ALL)
        endif()
    endif()
endif()
if (SIMRAIL)
	add_definitions(-DSIMRAIL)
endif()
if (RADIO_CFM)
    add_definitions(-DRADIO_CFM)
endif()
if (WIN32)
    add_definitions(-D_WIN32_WINNT=0x602)
endif()
add_definitions(-DBASELINE=3)
add_subdirectory(EVC)
add_subdirectory(DMI)
