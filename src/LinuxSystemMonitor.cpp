#include "LinuxSystemMonitor.hpp"
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <string>

LinuxSystemMonitor::LinuxSystemMonitor(): infoStreamRAM("/proc/meminfo") {
    if(!infoStreamRAM.is_open()){
        throw std::runtime_error("Error accessing /proc/meminfo kernel file");
    }

    std::string data = goThroughMemInfo("MemTotal:");
    totalRam = parseMemInfo(data);

    if (totalRam <= 0) {
        throw std::runtime_error("Failed to initialize system monitor: Could not read MemTotal");
    }
}

std::string LinuxSystemMonitor::goThroughMemInfo(const std::string& key) {
    infoStreamRAM.clear();
    infoStreamRAM.seekg(0);
    std::string line;
    while (std::getline(infoStreamRAM, line)) {
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
    std::string data = goThroughMemInfo("MemAvailable:");
    return parseMemInfo(data) / 1024.0 / 1024.0;
}

std::vector<std::pair<int, double>> LinuxSystemMonitor::getCpuUsage() {
    return {};
}

double LinuxSystemMonitor::getStorageUsage(enum SystemMonitor::StorageType) {
    return 0.0;
}

double LinuxSystemMonitor::getGPUTemperature() {
    return 0.0;
}

std::pair<double, double> LinuxSystemMonitor::getNetworkUsage() {
    return {0.0, 0.0};
}
