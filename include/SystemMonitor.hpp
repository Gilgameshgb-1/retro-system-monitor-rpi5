#pragma once

#include <utility>
#include <vector>
#include <string>

class SystemMonitor {
    public:
        enum StorageType { // Mostly for Raspberry Pi, but can be extended for other systems
            SSD,
            SDCARD
        };

        struct ProcessInfo {
            int pid;
            std::string name;
            double cpuUsagePercent;
            double memoryMiB;
        };

        virtual ~SystemMonitor() = default;

        virtual std::vector<std::pair<int, double>> getCPUUsage() = 0; // Which CPU core and its usage percent
        virtual double getCPUTemperature() = 0;
        virtual double getAvailableRAM() = 0; 
        virtual double getStorageUsage(enum StorageType) = 0;  
        virtual double getGPUUsage() = 0;
        virtual double getGPUTemperature() = 0;
        virtual std::pair<double, double> getNetworkUsage() = 0;
        virtual std::vector<ProcessInfo> getTopProcesses(int count = 5) = 0;
};