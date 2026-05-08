#pragma once

#include <utility>
#include <vector>
#include <string>
#include <chrono>
#include <map>

#include "SystemMonitor.hpp"

class LinuxSystemMonitor : public SystemMonitor {
    private:
        double totalRam = 0.0;

        struct CpuSnapshot { 
            long idle, total; 
        };
        std::vector<CpuSnapshot> prevCpuSnapshots;

        struct NetworkSnapshot {
            long rx, tx;
            std::chrono::steady_clock::time_point time;
        };

        struct ProcEntry { 
            int pid; std::string name; long ticks; double memoryMiB; 
        };

        NetworkSnapshot prevNetworkSnapshot{};

        std::map<int, long> prevProcessTicks;
        long prevTotalCpuTicks = 0;

        double parseMemInfo(const std::string& line);
        std::string findLineInFile(const std::string& path, const std::string& key);
        long readTotalCpuTicks();
        std::vector<CpuSnapshot> readCpuSnapshots();
        std::vector<ProcEntry> readProcessEntries();
        std::pair<long, long> readNetworkBytes();

        void initCpuSnapshots();
        void initNetworkSnapshot();
        void initProcessSnapshots();

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
        std::vector<ProcessInfo> getTopProcesses(int count) override;
};
