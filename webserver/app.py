import json
import threading
import os
from flask import Flask, render_template
from flask_sock import Sock

app = Flask(__name__)
sock = Sock(app)

# Thread-safe shared state
dashboards = set()
dashboards_lock = threading.Lock()
latest_snapshot = None
latest_lock = threading.Lock()


@app.route("/")
def index():
    return render_template("dashboard.html")


@sock.route("/ws/producer")
def producer(ws):
    """C++ binary connects here and pushes JSON snapshots."""
    global latest_snapshot
    print(f"[producer] connected: {ws}")
    try:
        while True:
            data = ws.receive()
            if data is None:
                break

            with latest_lock:
                latest_snapshot = data

            # Broadcast to all dashboards
            dead = []
            with dashboards_lock:
                for client in dashboards:
                    try:
                        client.send(data)
                    except Exception:
                        dead.append(client)
                for d in dead:
                    dashboards.discard(d)
    except Exception as e:
        print(f"[producer] error: {e}")
    finally:
        print("[producer] disconnected")


@sock.route("/ws/dashboard")
def dashboard(ws):
    """Browser connects here to receive live snapshots."""
    print(f"[dashboard] connected: {ws}")
    with dashboards_lock:
        dashboards.add(ws)

    # Send the latest snapshot immediately so the page is not empty
    with latest_lock:
        if latest_snapshot is not None:
            try:
                ws.send(latest_snapshot)
            except Exception:
                pass

    try:
        while True:
            # We do not expect messages from dashboards, but we need to keep
            # the connection alive and detect disconnects
            msg = ws.receive()
            if msg is None:
                break
    except Exception as e:
        print(f"[dashboard] error: {e}")
    finally:
        with dashboards_lock:
            dashboards.discard(ws)
        print("[dashboard] disconnected")


if __name__ == "__main__":
    host = os.environ.get('FLASK_HOST', '0.0.0.0')
    port = int(os.environ.get('FLASK_PORT', '5002'))
    app.run(host=host, port=port, debug=False)
