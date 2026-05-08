# retro-system-monitor-rpi5

A C++ system monitor targeting Linux, developed for the Raspberry Pi 5. Intended to feed a WebSocket for remote display (e.g. on a phone connected to the Pi), alongside other components in the Yocto repo.

## What it monitors

All data is read directly from Linux kernel virtual filesystems (`/proc`, `/sys`).

| Metric | Source | Notes |
|---|---|---|
| Available RAM | `/proc/meminfo` | GiB |
| CPU usage per core | `/proc/stat` | % per core, snapshot diff |
| CPU temperature | `/sys/class/hwmon` | AMD: Tccd1 sensor |
| GPU usage | `/sys/class/drm` | AMD only |
| GPU temperature | `/sys/class/hwmon` | AMD only |
| Storage usage | `statvfs` | % used, SSD or SD card |
| Network usage | `/proc/net/dev` | MiB/s up/down, all non-loopback interfaces |
| Top processes | `/proc/[pid]/stat` | CPU % + memory (MiB), snapshot diff |

## Architecture

```
SystemMonitor          (abstract interface - include/SystemMonitor.hpp)
└── LinuxSystemMonitor (Linux implementation - src/LinuxSystemMonitor.cpp)
```

`SystemMonitor` defines the interface. Platform-specific classes implement it - `LinuxSystemMonitor` for a desktop Linux machine, with a `RPiSystemMonitor` planned for the Pi 5.

Snapshot-based metrics (CPU usage, network, top processes) seed their initial state in the constructor and diff on each call - callers should wait at least 1 second between construction and the first read for meaningful results.

## Build

```bash
cmake -B build -S .
cmake --build build
./build/retro-system-monitor
```

## Requirements

- Linux
- CMake 3.16+
- C++17

## Hardware notes

Sensor paths are currently hardcoded for the development machine (AMD CPU/GPU). These will differ on RPi5 and will be handled by a dedicated `RPiSystemMonitor` class.

| Sensor | Path |
|---|---|
| CPU temp (AMD Tccd1) | `/sys/class/hwmon/hwmon2/temp3_input` |
| GPU temp (AMD) | `/sys/class/hwmon/hwmon4/temp1_input` |
| GPU usage (AMD) | `/sys/class/drm/card1/device/gpu_busy_percent` |
| RPi5 temp | `/sys/class/thermal/thermal_zone0/temp` |
