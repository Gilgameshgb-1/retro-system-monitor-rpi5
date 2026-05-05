#pragma once

#include <utility>
#include <vector>

class SystemMonitor {
    public:
        enum StorageType { // Mostly for Raspberry Pi, but can be extended for other systems
            SSD,
            SDCARD
        };

        virtual ~SystemMonitor() = default;

        virtual std::vector<std::pair<int, double>> getCpuUsage() = 0; // Which CPU core and its usage percent
        virtual double getAvailableRAM() = 0; 
        virtual double getStorageUsage(enum StorageType) = 0;  
        virtual double getGPUTemperature() = 0;
        virtual std::pair<double, double> getNetworkUsage() = 0;
};