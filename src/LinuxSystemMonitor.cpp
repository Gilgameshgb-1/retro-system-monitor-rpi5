#include "LinuxSystemMonitor.hpp"
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

LinuxSystemMonitor::LinuxSystemMonitor() {
    totalRam = parseMemInfo(findLineInFile("/proc/meminfo", "MemTotal:"));
    if (totalRam <= 0)
        throw std::runtime_error("Failed to initialize system monitor: Could not read MemTotal");

    initCpuSnaphots();
}

std::string LinuxSystemMonitor::findLineInFile(const std::string& path, const std::string& key) {
    std::ifstream f(path);
    if (!f.is_open())
        throw std::runtime_error("Error accessing " + path);
    std::string line;
    while (std::getline(f, line)) {
        if (line.find(key) == 0) return line;
    }
    return "";
}

double LinuxSystemMonitor::parseMemInfo(const std::string& line) {
    try {
        return std::stod(line.substr(line.find_first_of("0123456789")));
    } catch (const std::exception& e) {
        std::cerr << "Error parsing memory info: " << e.what() << std::endl;
        return 0.0;
    }
}

double LinuxSystemMonitor::getAvailableRAM() {
    std::string data = findLineInFile("/proc/meminfo", "MemAvailable:");
    return parseMemInfo(data) / 1024.0 / 1024.0;
}

void LinuxSystemMonitor::initCpuSnaphots() {
    std::ifstream f("/proc/stat");
    if(!f.is_open()){
        throw std::runtime_error("Error accessing CPU usage kernel file");
    }

    std::string line;
    while (std::getline(f, line)) {
        if(line.find("cpu") == 0)
        {
            std::string labelCpu;
            std::istringstream ss(line); 
            long user, nice, system, idle, iowait, irq, softirq, steal;
            ss >> labelCpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

            if(labelCpu == "cpu") continue; // Skip first CPU line (total usage)
            
            prevCpuSnapshots.push_back({idle + iowait, user + nice + system + idle + iowait + irq + softirq + steal});
        }
    }
}

std::vector<std::pair<int, double>> LinuxSystemMonitor::getCpuUsage() {
    std::ifstream f("/proc/stat");
    if(!f.is_open()){
        throw std::runtime_error("Error accessing CPU usage kernel file");
    }

    long sumOfCPUTime = 0;
    std::string line;
    std::vector<cpuSnapshot> snapshots;

    while (std::getline(f, line)) {
        if(line.find("cpu") == 0)
        {
            std::string labelCpu;
            std::istringstream ss(line); // Take out each streamed line and parse by space what we want
            long user, nice, system, idle, iowait, irq, softirq, steal;
            ss >> labelCpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

            if(labelCpu == "cpu") continue; // Skip first CPU line (total usage)
            
            snapshots.push_back({idle + iowait, user + nice + system + idle + iowait + irq + softirq + steal});
        }
    }
    std::vector<std::pair<int, double>> usagePercentages;
    for (int i = 0; i < snapshots.size(); ++i) {
        long deltaIdle  = snapshots[i].idle  - prevCpuSnapshots[i].idle;
        long deltaTotal = snapshots[i].total - prevCpuSnapshots[i].total;
        if (deltaTotal == 0) continue;
        double usage = (1.0 - static_cast<double>(deltaIdle) / deltaTotal) * 100.0;
        usagePercentages.push_back({i, usage});
    }
    prevCpuSnapshots = snapshots;
    return usagePercentages;
}

double LinuxSystemMonitor::getStorageUsage(enum SystemMonitor::StorageType) {
    return 0.0;
}

double LinuxSystemMonitor::getGPUTemperature() {
    std::ifstream f("/sys/class/hwmon/hwmon4/temp1_input"); //AMD GPU on my system
    if (!f.is_open()){
        throw std::runtime_error("Error accessing GPU temperature kernel file");
    }
    std::string line;
    std::getline(f, line);

    return std::stod(line) / 1000.0; 
}

std::pair<double, double> LinuxSystemMonitor::getNetworkUsage() {
    return {0.0, 0.0};
}
