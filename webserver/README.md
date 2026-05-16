# retro-system-monitor webserver

Flask websocket server + mobile-focused dashboard for the C++ system monitor.

## Architecture

```
[C++ binary] --ws--> /ws/producer --> [Flask broker] --> /ws/dashboard --ws--> [Browser]
```

- Producer (C++) connects to `/ws/producer` and pushes JSON snapshots
- Browser connects to `/ws/dashboard` and receives broadcasts
- Server caches the latest snapshot so new browser connections get data immediately

The server is target-agnostic: any process that can speak WebSocket and produce the agreed JSON shape can be the producer.

## Run

```bash
cd webserver
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
python app.py
```

Open `http://localhost:5000` on your laptop, or `http://<pi-ip>:5000` on your phone.

## Testing without the C++ binary

You can simulate the producer with a Python one-liner:

```bash
python3 -c "
import json, time, math, websocket
ws = websocket.create_connection('ws://localhost:5000/ws/producer')
i = 0
while True:
    payload = {
        'timestamp': int(time.time()),
        'host': 'test',
        'ram': {'available_gib': 4 + math.sin(i/10), 'total_gib': 8.0},
        'cpu': {'cores': [50 + 30*math.sin(i/5 + c) for c in range(8)], 'temp_c': 55.0},
        'gpu': {'usage_percent': abs(50 * math.sin(i/8)), 'temp_c': 48.0},
        'storage': {'ssd_used_percent': 42.0},
        'network': {'down_mib_s': abs(math.sin(i/3)), 'up_mib_s': abs(math.cos(i/3))},
        'top_processes': [{'pid': 1, 'name': 'firefox', 'cpu_percent': 5.1, 'memory_mib': 432.0}]
    }
    ws.send(json.dumps(payload))
    time.sleep(1)
    i += 1
"
```

(needs `pip install websocket-client`)

## JSON contract

```json
{
  "timestamp": 1716000000,
  "host": "raspberrypi5",
  "ram": { "available_gib": 2.1, "total_gib": 8.0 },
  "cpu": { "cores": [12.3, 5.1], "temp_c": 55.2 },
  "gpu": { "usage_percent": 27, "temp_c": 48.0 },
  "storage": { "ssd_used_percent": 42.0 },
  "network": { "down_mib_s": 0.5, "up_mib_s": 0.1 },
  "top_processes": [
    { "pid": 123, "name": "firefox", "cpu_percent": 5.1, "memory_mib": 432.0 }
  ]
}
```

All top-level keys are optional. Missing keys mean that card is hidden in the dashboard.
