#pragma once

#include <utility>
#include <vector>
#include <string>
#include <fstream>
#include <chrono>

#include "SystemMonitor.hpp"

class LinuxSystemMonitor : public SystemMonitor{
    private:
        double totalRam = 0.0;

        struct cpuSnapshot{
            long idle;
            long total;
        };

        std::vector<cpuSnapshot> prevCpuSnapshots;

        struct networkSnapshot {
            long rx, tx;
            std::chrono::steady_clock::time_point time;
        };
        networkSnapshot prevNetworkSnapshot{};

        double parseMemInfo(const std::string& line);
        std::string findLineInFile(const std::string& path, const std::string& key);

        void initCpuSnaphots();
        void initNetworkSnapshot();
        std::pair<long, long> readNetworkBytes();

    public:
        LinuxSystemMonitor();
        ~LinuxSystemMonitor() = default;

        std::vector<std::pair<int, double>> getCPUUsage() override;
        double getCPUTemperature() override;
        double getAvailableRAM() override; 
        double getStorageUsage(enum SystemMonitor::StorageType) override;  
        double getGPUTemperature() override;
        double getGPUUsage() override;
        std::pair<double, double> getNetworkUsage() override;
};