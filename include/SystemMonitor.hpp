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

        // some of these functions actually mutate the state of the object
        // strictly speaking I decided to only make const the methods
        // i am sure do not mutate the state

        virtual std::vector<std::pair<int, double>> getCPUUsage() = 0; // Which CPU core and its usage percent
        virtual double getCPUTemperature() const = 0; 
        virtual double getRamSizeGiB() const = 0;
        virtual double getAvailableRAM() const = 0; 
        virtual double getStorageUsage(enum StorageType) const = 0;  
        virtual double getGPUUsage() const = 0;
        virtual double getGPUTemperature() const = 0;
        virtual std::pair<double, double> getNetworkUsage() = 0;
        virtual std::vector<ProcessInfo> getTopProcesses(int count = 5) = 0;
};