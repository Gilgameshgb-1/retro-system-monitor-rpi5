#include "RpiSystemMonitor.hpp"
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <sys/statvfs.h>

RpiSystemMonitor::RpiSystemMonitor() {
    totalRam = parseMemInfo(findLineInFile("/proc/meminfo", "MemTotal:"));
    if (totalRam <= 0)
        throw std::runtime_error("Failed to initialize system monitor: Could not read MemTotal");

    initCpuSnapshots();
    initNetworkSnapshot();
    initProcessSnapshots();
}

std::string RpiSystemMonitor::findLineInFile(const std::string& path, const std::string& key) {
    std::ifstream f(path);
    if (!f.is_open())
        throw std::runtime_error("Error accessing " + path);
    std::string line;
    while (std::getline(f, line)) {
        if (line.find(key) == 0) return line;
    }
    return "";
}

double RpiSystemMonitor::parseMemInfo(const std::string& line) {
    try {
        return std::stod(line.substr(line.find_first_of("0123456789")));
    } catch (const std::exception& e) {
        std::cerr << "Error parsing memory info: " << e.what() << std::endl;
        return 0.0;
    }
}

long RpiSystemMonitor::readTotalCpuTicks() const {
    std::ifstream f("/proc/stat");
    if (!f.is_open())
        throw std::runtime_error("Error accessing /proc/stat");
    std::string line;
    std::getline(f, line);
    std::istringstream ss(line);
    std::string label;
    long user, nice, system, idle, iowait, irq, softirq, steal;
    ss >> label >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
    return user + nice + system + idle + iowait + irq + softirq + steal;
}

std::vector<RpiSystemMonitor::CpuSnapshot> RpiSystemMonitor::readCpuSnapshots() const {
    std::ifstream f("/proc/stat");
    if (!f.is_open())
        throw std::runtime_error("Error accessing /proc/stat");

    std::vector<CpuSnapshot> snapshots;
    std::string line;
    while (std::getline(f, line)) {
        if (line.find("cpu") != 0) break;
        std::string label;
        std::istringstream ss(line);
        long user, nice, system, idle, iowait, irq, softirq, steal;
        ss >> label >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
        if (label == "cpu") continue;
        snapshots.push_back({idle + iowait, user + nice + system + idle + iowait + irq + softirq + steal});
    }
    return snapshots;
}

std::vector<RpiSystemMonitor::ProcEntry> RpiSystemMonitor::readProcessEntries() const {
    std::vector<ProcEntry> entries;
    for (const auto& folder : std::filesystem::directory_iterator("/proc")) {
        std::string name = folder.path().filename().string();
        if (!std::filesystem::is_directory(folder)) continue;
        if (name.empty() || !std::all_of(name.begin(), name.end(), ::isdigit)) continue;

        std::ifstream f("/proc/" + name + "/stat");
        if (!f.is_open()) continue;
        std::string line;
        std::getline(f, line);

        size_t nameStart = line.find('(');
        size_t nameEnd   = line.rfind(')');
        std::string procName = line.substr(nameStart + 1, nameEnd - nameStart - 1);

        std::istringstream ss(line.substr(nameEnd + 2));
        char state;
        long ignore, utime, stime;
        ss >> state >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> utime >> stime;

        double memoryMiB = 0.0;
        std::ifstream statusFile("/proc/" + name + "/status");
        std::string statusLine;
        while (std::getline(statusFile, statusLine)) {
            if (statusLine.find("VmRSS:") == 0) {
                std::istringstream mss(statusLine);
                std::string label;
                long kb;
                mss >> label >> kb;
                memoryMiB = kb / 1024.0;
                break;
            }
        }

        entries.push_back({std::stoi(name), procName, utime + stime, memoryMiB});
    }
    return entries;
}

double RpiSystemMonitor::getAvailableRAM() const {
    std::string data = findLineInFile("/proc/meminfo", "MemAvailable:");
    return parseMemInfo(data) / 1024.0 / 1024.0;
}

double RpiSystemMonitor::getRamSizeGiB() const {
    return totalRam / 1024.0 / 1024.0;
}

void RpiSystemMonitor::initCpuSnapshots() {
    prevCpuSnapshots = readCpuSnapshots();
}

std::vector<std::pair<int, double>> RpiSystemMonitor::getCPUUsage() {
    std::vector<CpuSnapshot> snapshots = readCpuSnapshots();
    std::vector<std::pair<int, double>> usagePercentages;
    for (int i = 0; i < (int)snapshots.size(); ++i) {
        long deltaIdle  = snapshots[i].idle  - prevCpuSnapshots[i].idle;
        long deltaTotal = snapshots[i].total - prevCpuSnapshots[i].total;
        if (deltaTotal == 0) continue;
        double usage = (1.0 - static_cast<double>(deltaIdle) / deltaTotal) * 100.0;
        usagePercentages.push_back({i, usage});
    }
    prevCpuSnapshots = snapshots;
    return usagePercentages;
}

double RpiSystemMonitor::getCPUTemperature() const{
    std::ifstream f("/sys/class/thermal/thermal_zone0/temp");
    if (!f.is_open())
        throw std::runtime_error("Error accessing CPU temperature kernel file");
    std::string line;
    std::getline(f, line);
    return std::stod(line) / 1000.0;
}

double RpiSystemMonitor::getStorageUsage(SystemMonitor::StorageType type) const {
    if (type != SystemMonitor::SSD) return 0.0;

    struct statvfs stat;
    if (statvfs("/", &stat) != 0) return 0.0;

    double total = stat.f_blocks * stat.f_frsize;
    double free  = stat.f_bfree  * stat.f_frsize;
    return (total - free) / total * 100.0;
}
// The same SoC is used for both CPU and GPU, so its quite redundant
// TODO: Either try to separate somehow or just remove the GPU temp from
// the base class inherited by platforms
double RpiSystemMonitor::getGPUTemperature() const {
    std::ifstream f("/sys/class/thermal/thermal_zone0/temp");
    if (!f.is_open())
        throw std::runtime_error("Error accessing GPU temperature kernel file");
    std::string line;
    std::getline(f, line);
    return std::stod(line) / 1000.0;
}

// Unfortunately kernel files for this don't exist in the kernel im using
// From the forums: 
// Gordon Hollingworth PhD
// Raspberry Pi - Chief Technology Officer - Software
// ------
// The GPU usage is available in lxtask, it does:
// grep -srI drm-engine /proc/*/fdinfo/*
// That will tell you what amount of binning and rendering is done by that process
// So we will adapt the method accordingly

long long RpiSystemMonitor::getTotalGPUTimeNs() const {
    long long totalNs = 0;
    std::error_code ec;

    // Use std::error_code to safely handle files or directories disappearing mid-loop
    for (const auto& procEntry : std::filesystem::directory_iterator("/proc", ec)) {
        if (ec || !procEntry.is_directory()) continue;

        std::string pidStr = procEntry.path().filename().string();
        if (!std::all_of(pidStr.begin(), pidStr.end(), ::isdigit)) continue;

        std::string fdinfoPath = "/proc/" + pidStr + "/fdinfo";
        
        std::error_code fdEc;
        if (!std::filesystem::exists(fdinfoPath, fdEc)) continue;

        for (const auto& fdEntry : std::filesystem::directory_iterator(fdinfoPath, fdEc)) {
            if (fdEc) break;

            std::ifstream f(fdEntry.path());
            if (!f.is_open()) continue;

            std::string line;
            while (std::getline(f, line)) {
                // VideoCore DRM engine stats export under keys matching 'drm-engine-'
                if (line.find("drm-engine-") == 0) {
                    size_t colon = line.find(':');
                    if (colon != std::string::npos) {
                        try {
                            totalNs += std::stoll(line.substr(colon + 1));
                        } catch (...) {
                            // Mmmm skip skip errors
                        }
                    }
                }
            }
        }
    }
    return totalNs;
}

double RpiSystemMonitor::getGPUUsage() const {
auto currentTime = std::chrono::steady_clock::now();
    long long currentGPUTimeNs = getTotalGPUTimeNs();

    if (prevGPUTimeNs == 0) {
        // Cold start initialization
        prevGPUTimeNs = currentGPUTimeNs;
        prevSampleTime = currentTime;
        return 0.0;
    }

    // Calculate elapsed system (wall) time in nanoseconds
    auto elapsedWallTimeNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
        currentTime - prevSampleTime
    ).count();

    long long deltaGPUTimeNs = currentGPUTimeNs - prevGPUTimeNs;

    double usagePercent = 0.0;
    if (elapsedWallTimeNs > 0 && deltaGPUTimeNs >= 0) {
        usagePercent = (static_cast<double>(deltaGPUTimeNs) / static_cast<double>(elapsedWallTimeNs)) * 100.0;
    }

    // Update tracking baselines
    prevGPUTimeNs = currentGPUTimeNs;
    prevSampleTime = currentTime;

    // Clamp the percentage between 0 and 100 to catch math overflows or concurrency anomalies
    return std::clamp(usagePercent, 0.0, 100.0);
}

std::pair<long, long> RpiSystemMonitor::readNetworkBytes() const {
    std::ifstream f("/proc/net/dev");
    if (!f.is_open())
        throw std::runtime_error("Error accessing /proc/net/dev");

    long totalRx = 0, totalTx = 0;
    std::string line;
    std::getline(f, line);
    std::getline(f, line);

    while (std::getline(f, line)) {
        std::istringstream ss(line);
        std::string iface;
        long rx, tx, ignore;
        ss >> iface >> rx >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> tx;
        if (iface == "lo:") continue;
        totalRx += rx;
        totalTx += tx;
    }
    return {totalRx, totalTx};
}

void RpiSystemMonitor::initNetworkSnapshot() {
    auto [rx, tx] = readNetworkBytes();
    prevNetworkSnapshot = {rx, tx, std::chrono::steady_clock::now()};
}

std::pair<double, double> RpiSystemMonitor::getNetworkUsage() {
    auto [rx, tx] = readNetworkBytes();
    auto now = std::chrono::steady_clock::now();

    double seconds  = std::chrono::duration<double>(now - prevNetworkSnapshot.time).count();
    double download = (rx - prevNetworkSnapshot.rx) / seconds / (1024.0 * 1024.0);
    double upload   = (tx - prevNetworkSnapshot.tx) / seconds / (1024.0 * 1024.0);

    prevNetworkSnapshot = {rx, tx, now};
    return {download, upload};
}

void RpiSystemMonitor::initProcessSnapshots() {
    prevTotalCpuTicks = readTotalCpuTicks();
    for (const auto& folder : std::filesystem::directory_iterator("/proc")) {
        std::string name = folder.path().filename().string();
        if (!std::filesystem::is_directory(folder)) continue;
        if (name.empty() || !std::all_of(name.begin(), name.end(), ::isdigit)) continue;

        std::ifstream f("/proc/" + name + "/stat");
        if (!f.is_open()) continue;
        std::string line;
        std::getline(f, line);

        size_t nameEnd = line.rfind(')');
        std::istringstream ss(line.substr(nameEnd + 2));
        char state;
        long ignore, utime, stime;
        ss >> state >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> utime >> stime;
        prevProcessTicks[std::stoi(name)] = utime + stime;
    }
}

std::vector<SystemMonitor::ProcessInfo> RpiSystemMonitor::getTopProcesses(int count) {
    long totalCpuTicks = readTotalCpuTicks();
    long deltaTotalCpu = totalCpuTicks - prevTotalCpuTicks;

    std::vector<SystemMonitor::ProcessInfo> processes;
    std::map<int, long> currentProcessTicks;

    for (const auto& e : readProcessEntries()) {
        currentProcessTicks[e.pid] = e.ticks;

        long prev = 0;
        if (prevProcessTicks.count(e.pid))
            prev = prevProcessTicks[e.pid];

        double cpuPercent = 0.0;
        if (deltaTotalCpu > 0)
            cpuPercent = (static_cast<double>(e.ticks - prev) / deltaTotalCpu) * 100.0;

        processes.push_back({e.pid, e.name, cpuPercent, e.memoryMiB});
    }

    prevTotalCpuTicks = totalCpuTicks;
    prevProcessTicks  = currentProcessTicks;

    std::sort(processes.begin(), processes.end(), [](const auto& a, const auto& b) {
        return a.cpuUsagePercent > b.cpuUsagePercent;
    });
    processes.resize(std::min(processes.size(), static_cast<size_t>(count)));

    return processes;
}
