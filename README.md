# retro-system-monitor-rpi5

A C++ system monitor for the Raspberry Pi 5. Probably connect to a ws to see via phone connected to pi like other things in the yocto repo.

## What it does

Reads live system stats from the Linux kernel:

- Available RAM (GiB)
- CPU usage per core
- Storage usage (SSD / SD card)
- GPU temperature
- Network usage (up / down)

## Build

```bash
cmake -B build -S .
cmake --build build
./build/retro-system-monitor
```

## Requirements

- Linux (reads from `/proc`)
- CMake 3.16+
- C++17

## Status

Early stages. RAM reading is working. CPU, storage, GPU, and network are not yet implemented.
