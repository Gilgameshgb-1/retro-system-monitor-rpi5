#include <iostream>
#include <chrono>
#include <thread>
#include <ctime>
#include <unistd.h>
#include <ixwebsocket/IXWebSocket.h>
#include <nlohmann/json.hpp>
#include "LinuxSystemMonitor.hpp"

int main() {
    LinuxSystemMonitor monitor;

    char host[256];
    gethostname(host, sizeof(host));

    ix::WebSocket ws;
    ws.setUrl("ws://localhost:5000/ws/producer");
    ws.setOnMessageCallback([](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Open)
            std::cout << "ws: connected" << std::endl;
        else if (msg->type == ix::WebSocketMessageType::Close)
            std::cout << "ws: closed" << std::endl;
        else if (msg->type == ix::WebSocketMessageType::Error)
            std::cout << "ws: error: " << msg->errorInfo.reason << std::endl;
    });
    ws.start();

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        nlohmann::json snapshot;
        snapshot["timestamp"] = std::time(nullptr);
        snapshot["host"]      = host;

        snapshot["ram"]["available_gib"] = monitor.getAvailableRAM();
        snapshot["ram"]["total_gib"]     = monitor.getRamSizeGiB();

        nlohmann::json cores = nlohmann::json::array();
        for (const auto& [idx, usage] : monitor.getCPUUsage())
            cores.push_back(usage);
        snapshot["cpu"]["cores"]  = cores;
        snapshot["cpu"]["temp_c"] = monitor.getCPUTemperature();

        snapshot["gpu"]["usage_percent"] = monitor.getGPUUsage();
        snapshot["gpu"]["temp_c"]        = monitor.getGPUTemperature();

        snapshot["storage"]["ssd_used_percent"] = monitor.getStorageUsage(SystemMonitor::SSD);

        auto [down, up] = monitor.getNetworkUsage();
        snapshot["network"]["down_mib_s"] = down;
        snapshot["network"]["up_mib_s"]   = up;

        nlohmann::json procs = nlohmann::json::array();
        for (const auto& p : monitor.getTopProcesses(5)) {
            procs.push_back({
                {"pid", p.pid},
                {"name", p.name},
                {"cpu_percent", p.cpuUsagePercent},
                {"memory_mib", p.memoryMiB}
            });
        }
        snapshot["top_processes"] = procs;

        if (ws.getReadyState() == ix::ReadyState::Open)
            ws.send(snapshot.dump());
    }

    ws.stop();
    return 0;
}
