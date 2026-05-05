#pragma once

#include <utility>
#include <vector>
#include <string>
#include <fstream>

#include "SystemMonitor.hpp"

class LinuxSystemMonitor : public SystemMonitor{
    private:
        double totalRam = 0.0;
        double availableRam = 0.0;
        std::ifstream infoStreamRAM;

        double parseMemInfo(const std::string& line);
        std::string goThroughMemInfo(const std::string& key);

    public:
        LinuxSystemMonitor();
        ~LinuxSystemMonitor() = default;

        std::vector<std::pair<int, double>> getCpuUsage() override;
        double getAvailableRAM() override; 
        double getStorageUsage(enum SystemMonitor::StorageType) override;  
        double getGPUTemperature() override;
        std::pair<double, double> getNetworkUsage() override;
};