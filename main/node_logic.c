#include "node_logic.h"

void start_heartbeat() {
    start_timer_periodic(heartbeat_timer, "heartbeat timer", HEARTBEAT_INTERVAL_US);
    start_timer_once(timeout_timer, "timeout timer", TIMEOUT_US);
}

void start_sync() {
    send_sync_request();
    start_timer_periodic(sync_timer, "sync timer", SYNC_INTERVAL_US);
}

void start_leader_election() {
    if (!node_is_idle()) return;
    ESP_LOGI(NODE_LOGIC_TAG, "Starting leader election...");

    mesh_publish_leader_election();

    start_timer_once(leader_victory_timer, "leader_victory_timer", LEADER_ELECTION_TIMEOUT_US);
}