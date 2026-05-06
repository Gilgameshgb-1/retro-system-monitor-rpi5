#pragma once

#include <utility>
#include <vector>
#include <string>
#include <fstream>

#include "SystemMonitor.hpp"

class LinuxSystemMonitor : public SystemMonitor{
    private:
        double totalRam = 0.0;

        struct cpuSnapshot{
            long idle;
            long total;
        };

        std::vector<cpuSnapshot> prevCpuSnapshots;

        double parseMemInfo(const std::string& line);
        std::string findLineInFile(const std::string& path, const std::string& key);

        void initCpuSnaphots(); //We snapshot the CPU at the construction of the object

    public:
        LinuxSystemMonitor();
        ~LinuxSystemMonitor() = default;

        std::vector<std::pair<int, double>> getCpuUsage() override;
        double getAvailableRAM() override; 
        double getStorageUsage(enum SystemMonitor::StorageType) override;  
        double getGPUTemperature() override;
        std::pair<double, double> getNetworkUsage() override;
};