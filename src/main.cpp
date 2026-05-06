#include <iostream>
#include <iomanip>
#include <unistd.h>
#include "LinuxSystemMonitor.hpp"

int main() {
    //Example main not reflective of what actually is suopposed to happen in main
    LinuxSystemMonitor monitor;
    std::cout << "Available RAM: " << monitor.getAvailableRAM() << " GiB\n";
    std::cout << "GPU Temp:      " << monitor.getGPUTemperature() << " C\n";

    sleep(1);

    std::cout << "\nCPU Usage:\n";
    for (auto& [core, usage] : monitor.getCpuUsage()) {
        std::cout << "  cpu" << core << ": "
                  << std::fixed << std::setprecision(1) << std::setw(5) << usage << "%\n";
    }

    return 0;
}