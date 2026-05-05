cmake_minimum_required(VERSION 3.16)
project(retro-system-monitor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_THERMAL "Enable thermal sensor support" OFF)

include_directories(include)

add_executable(retro-system-monitor
    src/main.cpp
    src/LinuxSystemMonitor.cpp
)

if(ENABLE_THERMAL)
    target_compile_definitions(retro-system-monitor PRIVATE ENABLE_THERMAL)
endif()