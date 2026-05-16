cmake_minimum_required(VERSION 3.16)
project(retro-system-monitor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_THERMAL "Enable thermal sensor support" OFF)

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

add_executable(retro-system-monitor
    src/main.cpp
    src/LinuxSystemMonitor.cpp
)

target_link_libraries(retro-system-monitor PRIVATE ixwebsocket nlohmann_json::nlohmann_json)

if(ENABLE_THERMAL)
    target_compile_definitions(retro-system-monitor PRIVATE ENABLE_THERMAL)
endif()