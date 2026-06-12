#pragma once

#include <utility>
#include <vector>
#include <string>
#include <chrono>
#include <map>

#include "SystemMonitor.hpp"

class RpiSystemMonitor : public SystemMonitor {
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

        // For tracking GPU usage we add new params
        // ugly hack for const functions
        mutable long long prevGPUTimeNs = 0;
        mutable std::chrono::steady_clock::time_point prevSampleTime;
        long long getTotalGPUTimeNs() const;

        static double parseMemInfo(const std::string& line);
        static std::string findLineInFile(const std::string& path, const std::string& key);
        long readTotalCpuTicks() const;
        std::vector<CpuSnapshot> readCpuSnapshots() const;
        std::vector<ProcEntry> readProcessEntries() const;
        std::pair<long, long> readNetworkBytes() const;

        void initCpuSnapshots();
        void initNetworkSnapshot();
        void initProcessSnapshots();

    public:
        RpiSystemMonitor();
        ~RpiSystemMonitor() = default;

        std::vector<std::pair<int, double>> getCPUUsage() override;
        double getCPUTemperature() const override;
        double getRamSizeGiB() const override;
        double getAvailableRAM() const override;
        double getStorageUsage(enum SystemMonitor::StorageType) const override;
        double getGPUTemperature() const override;
        double getGPUUsage() const override;
        std::pair<double, double> getNetworkUsage() override;
        std::vector<ProcessInfo> getTopProcesses(int count) override;
};
