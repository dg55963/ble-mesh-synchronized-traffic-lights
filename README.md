**BLE Mesh Synchronized Traffic Lights**

Distributed ESP32-based traffic intersection simulator using BLE Mesh.

- **What it is:** A distributed system of ESP32 nodes that form a BLE Mesh and coordinate to simulate a synchronized traffic intersection. Each node controls a traffic-light module with three LEDs and participates in leader election, time synchronization, and distributed action coordination.

**Key Features**

- **Distributed coordination:** Nodes dynamically join a mesh and coordinate actions in real time.
- **Time synchronization:** Robust synchronization across nodes despite network delays and imperfect transmission.
- **Leader election:** Lightweight leader election algorithm used to coordinate global decisions.
- **Fault tolerance:** Handles node failures and dynamic topology changes gracefully.
- **BLE Mesh model:** Uses BLE Mesh for state exchange and coordination after provisioning.
- **Demo application:** Simulates a traffic light intersection as a concrete demonstration of distributed coordination.

**Why it matters**

- Demonstrates how constrained IoT devices can run resilient distributed algorithms (synchronization, leader election) and cooperate using BLE Mesh — a useful reference for research, demos, and embedded systems interviews.

**Quick start (build & flash)**

- Setup: Install ESP-IDF and toolchain for your platform (see ESP-IDF docs).
- Build and flash (from project root):

```bash
idf.py build
idf.py -p <PORT> flash monitor
```

Replace `<PORT>` with your device serial port.

**Hardware**

- ESP32 development board per node.
- Traffic light module: 3 LEDs (red/yellow/green) connected to GPIOs.

**How it works (high level)**

- Nodes form a BLE Mesh and exchange state after provisioning.
- A leader is elected to coordinate global timing and resolve contention.
- Time synchronization messages keep nodes aligned despite latency.
- Each node runs a local traffic controller that follows the synchronized schedule and updates LEDs.

**Where to look in the code**

- Entry point: [main/main.c](main/main.c)
- Mesh handling: [main/mesh_handlers.c](main/mesh_handlers.c)
- Time sync logic: [main/synchronization.c](main/synchronization.c)
- Leader election & node logic: [main/node_logic.c](main/node_logic.c)
- Traffic controller & phases: [main/traffic_controller.c](main/traffic_controller.c)
- Timers & scheduling: [main/my_timers.c](main/my_timers.c)

These modules implement the distributed algorithms and the LED control used in the demo.

**Design notes (short)**

- The system prioritizes predictable, synchronized behavior over absolute precision — practical for lights where bounded drift and recovery are acceptable.
- BLE Mesh is used as the communication substrate: after provisioning, nodes exchange lightweight state messages rather than streaming large payloads.

**To demo locally**

1. Provision a few ESP32 nodes into the BLE Mesh (use a mobile provisioner or a PC-based tool).
2. Flash the firmware to each node.
3. Power the nodes and watch them elect a leader and synchronize — LEDs will show coordinated traffic phases.

**Want to contribute or evaluate me?**

- Suggestions: add a provisioning walkthrough, wiring diagrams, and a short video of a running demo.
- If you'd like, I can add a wiring diagram, schematic, or a short test harness for automated simulation of multiple nodes.

---

Made to showcase distributed algorithms and embedded systems engineering with BLE Mesh and ESP32.
