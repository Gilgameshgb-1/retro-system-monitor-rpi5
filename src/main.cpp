#include <iostream>
#include "LinuxSystemMonitor.hpp"

int main() {

    LinuxSystemMonitor monitor;
    std::cout << "Available RAM: " << monitor.getAvailableRAM() << " GiB" << std::endl;

    return 0;
}