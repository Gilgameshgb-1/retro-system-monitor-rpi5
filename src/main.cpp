#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include <thread>
#include <ixwebsocket/IXWebSocket.h>
#include "LinuxSystemMonitor.hpp"

//comment out old main
// int main() {
//     //Example main not reflective of what actually is suopposed to happen in main
//     LinuxSystemMonitor monitor;
//     std::cout << "Available RAM: " << std::fixed << std::setprecision(2) << monitor.getAvailableRAM() << " GiB" << std::endl;
//     std::cout << "GPU Temp:      " << std::fixed << std::setprecision(2) << monitor.getGPUTemperature() << " C" << std::endl;
//     std::cout << "GPU Usage:      " << std::fixed << std::setprecision(2) << monitor.getGPUUsage() << "%" << std::endl;
//     std::cout << "Storage Usage:  " << std::fixed << std::setprecision(2) << monitor.getStorageUsage(SystemMonitor::SSD) << "%" << std::endl;
//     sleep(1);

//     std::cout<<"CPU Temp:      " << std::fixed << std::setprecision(2) << monitor.getCPUTemperature() << " C" << std::endl;

//     std::cout << std::fixed << std::setprecision(2) << "CPU Usage:" << std::endl;
//     for (auto& [core, usage] : monitor.getCPUUsage()) {
//         std::cout << "  cpu" << core << ": "
//                   << std::fixed << std::setprecision(1) << std::setw(5) << usage << "%" << std::endl;
//     }

//     std::cout << std::fixed << std::setprecision(2) << "Network Usage: " 
//               << monitor.getNetworkUsage().first << " MiB/s down, "
//               << monitor.getNetworkUsage().second << " MiB/s up" << std::endl;

//     for (const auto& process: monitor.getTopProcesses(5))
//     {
//         std::cout<< "PID: " << process.pid << " Name: " << process.name << " CPU Usage: " << process.cpuUsagePercent << "%" << " Memory: "<< process.memoryMiB << "MiB" << std::endl;
//     }

//     return 0;
// }

int main(){
    LinuxSystemMonitor monitor;

    ix::WebSocket ws;
    ws.setUrl("ws://localhost:5000/ws/producer");

    ws.setOnMessageCallback([](const ix::WebSocketMessagePtr &msg){
        if(msg->type == ix::WebSocketMessageType::Open)
            std::cout << "Connected" << std::endl;
        else if(msg->type == ix::WebSocketMessageType::Error)
            std::cout << "Error: " << msg->errorInfo.reason << std::endl;
    });

    ws.start();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    while(true)
    {
        nlohmann::json snapshot = {
            {"timestamp", 676767},
            {"host", "test"},
            {"ram", {{"available_gib", monitor.getAvailableRAM()}, {"total_gib", monitor.getRamSizeGiB()}}}
        };

        ws.send(snapshot.dump());

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    ws.stop();
    
    return 0;
}