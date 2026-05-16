#include "node_state.h"

node_state_t g_node = {
    .role = NODE_IDLE,
    .my_addr = BLE_MESH_ADDR_UNASSIGNED,
    .leader_addr = BLE_MESH_ADDR_UNASSIGNED,
    .slave_count = 0,
    .slave_check_vector = 0,
};

void node_reset(void) {
    g_node.role = NODE_IDLE;
    g_node.leader_addr = BLE_MESH_ADDR_UNASSIGNED;
    node_clear_slaves();
}

void node_become_leader(void) {
    g_node.role = NODE_LEADER;
    g_node.leader_addr = g_node.my_addr;
    node_clear_slaves();
}

void node_become_follower(uint16_t leader_addr) {
    g_node.role = NODE_FOLLOWER;
    g_node.leader_addr = leader_addr;
}

bool node_is_idle(void) { return g_node.role == NODE_IDLE; }

bool node_is_leader(void) { return g_node.role == NODE_LEADER; }

bool node_has_leader(void) { return g_node.leader_addr != BLE_MESH_ADDR_UNASSIGNED; }

void node_add_slave(uint16_t addr) {
    if (g_node.slave_count >= MAX_NODES - 1) {
        ESP_LOGW(NODE_STATE_TAG, "Slave list is full, cannot add addr 0x%04x", addr);
        return;
    }

    if (node_slave_exists(addr)) {
        ESP_LOGI(NODE_STATE_TAG, "Slave addr 0x%04x already exists in slave list", addr);
        return;
    }

    g_node.slave_list[g_node.slave_count++] = addr;
    ESP_LOGI(NODE_STATE_TAG, "Slave addr 0x%04x added to slave list", addr);
}

bool node_slave_exists(uint16_t addr) {
    for (size_t i = 0; i < g_node.slave_count; i++) {
        if (g_node.slave_list[i] == addr) {
            return true;
        }
    }
    return false;
}

void node_clear_slaves(void) {
    g_node.slave_count = 0;
    g_node.slave_check_vector = 0;

    memset(g_node.slave_list, 0, sizeof(g_node.slave_list));
}