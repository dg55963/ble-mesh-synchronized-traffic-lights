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

**How to run the demo**

1. Choose how many ESP32 nodes you want in the mesh.
   - The total node count is configured in [main/node_state.h](main/node_state.h) with `NUM_OF_NODES`.
   - `NUM_OF_NODES` includes the leader node, so if you want 4 boards in the mesh, keep it at `4`.
   - Do not set it to `9` or higher, because the code defines `MAX_NODES 9`.

2. Build and flash the same firmware to every board.
   - If you change `NUM_OF_NODES`, rebuild before flashing the boards again.
   - Each node runs the same binary and decides its role at runtime.

3. Provision every board into the same BLE Mesh network.
   - The firmware stays in provisioning mode until it is provisioned.
   - Use a mobile provisioner.
   - Assign the same Net Key / App Key to all nodes in the network.

4. Bind the vendor model and subscribe it to the traffic groups.
   - The traffic controller publishes scene commands to two fixed group addresses:
     - `0xC000` for the first traffic group
     - `0xC001` for the second traffic group
   - Subscribe each node’s vendor model to the group addresses you want it to receive.
   - In practice, the boards that should react to synchronized traffic phases should subscribe to both `0xC000` and `0xC001`.

5. Power the nodes and wait for the mesh to settle.
   - After provisioning, the nodes elect a leader, exchange state, and start the synchronized traffic schedule automatically.
   - The firmware waits until the expected number of follower nodes is present before starting the scheduler.

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
---

Made to showcase distributed algorithms and embedded systems engineering with BLE Mesh and ESP32.
