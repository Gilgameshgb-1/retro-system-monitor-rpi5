cmake_minimum_required(VERSION 3.16)
project(retro-system-monitor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_THERMAL "Enable thermal sensor support" OFF)

# 1. Add the compilation hack option for local desktop testing
option(FORCE_RPI_BUILD "Force compile the Raspberry Pi 5 version for testing" OFF)

include(FetchContent)

FetchContent_Declare(
    ixwebsocket
    GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
    GIT_TAG v11.4.5
)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(ixwebsocket)
FetchContent_MakeAvailable(json)

include_directories(include)

# Detect platform for building
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm" OR FORCE_RPI_BUILD)
    message(STATUS ">>>> CONFIGURING FOR TARGET: Raspberry Pi 5 <<<<")
    set(MONITOR_SRC src/RpiSystemMonitor.cpp)
    set(PLATFORM_DEF "IS_RPI")
else()
    message(STATUS ">>>> CONFIGURING FOR TARGET: Generic Linux Desktop <<<<")
    set(MONITOR_SRC src/LinuxSystemMonitor.cpp)
    set(PLATFORM_DEF "IS_GENERIC_LINUX")
endif()

add_executable(retro-system-monitor
    src/main.cpp
    ${MONITOR_SRC}
)

target_link_libraries(retro-system-monitor PRIVATE ixwebsocket nlohmann_json::nlohmann_json)

target_compile_definitions(retro-system-monitor PRIVATE ${PLATFORM_DEF})

if(ENABLE_THERMAL)
    target_compile_definitions(retro-system-monitor PRIVATE ENABLE_THERMAL)
endif()