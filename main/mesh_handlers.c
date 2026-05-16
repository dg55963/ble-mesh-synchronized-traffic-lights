#include "mesh_handlers.h"

void handle_leader_election(esp_ble_mesh_model_cb_param_t* param) {
    if (param->model_operation.ctx->addr > g_node.my_addr) {
        if (node_is_leader()) {
            mesh_send_leader_victory(param->model_operation.ctx->net_idx, param->model_operation.ctx->app_idx, param->model_operation.ctx->addr);

        } else {
            mesh_send_leader_alive(param->model_operation.ctx->net_idx, param->model_operation.ctx->app_idx, param->model_operation.ctx->addr);
        }
    } else if (param->model_operation.ctx->addr < g_node.leader_addr && g_node.leader_addr != BLE_MESH_ADDR_UNASSIGNED) {
        stop_timer(heartbeat_timer);
        stop_timer(timeout_timer);
    }
}

void handle_leader_alive(esp_ble_mesh_model_cb_param_t* param) {
    if (param->model_operation.ctx->addr < g_node.my_addr) {
        stop_timer(leader_victory_timer);
    }
}

void handle_leader_victory(esp_ble_mesh_model_cb_param_t* param) {
    if (param->model_operation.ctx->addr < g_node.my_addr) {
        stop_timer(leader_victory_timer);
        node_become_follower(param->model_operation.ctx->addr);
        mesh_send_leader_ack(param->model_operation.ctx->net_idx, param->model_operation.ctx->app_idx, param->model_operation.ctx->addr);
    }
}

void handle_leader_ack(esp_ble_mesh_model_cb_param_t* param) { node_add_slave(param->model_operation.ctx->addr); }

void handle_heartbeat(esp_ble_mesh_model_cb_param_t* param) {
    if (node_has_leader() && param->model_operation.ctx->addr == g_node.leader_addr) {
        mesh_send_heartbeat_ack(param->model_operation.ctx->net_idx, param->model_operation.ctx->app_idx, param->model_operation.ctx->addr);
        start_timer_once(timeout_timer, "timeout timer", TIMEOUT_US);
    }
}

void handle_heartbeat_ack(esp_ble_mesh_model_cb_param_t* param) {
    if (node_is_leader()) {
        for (size_t i = 0; i < g_node.slave_count; i++) {
            if (g_node.slave_list[i] == param->model_operation.ctx->addr) {
                ESP_LOGI(MESH_HANDLERS_TAG, "Received heartbeat ACK from slave addr 0x%04x", param->model_operation.ctx->addr);
                g_node.slave_check_vector |= (1 << i);
                break;
            }
        }
        if (g_node.slave_count > 0 && g_node.slave_check_vector == ((1 << g_node.slave_count) - 1)) {
            ESP_LOGI(MESH_HANDLERS_TAG, "Received heartbeat ACK from all slaves");
            start_timer_once(timeout_timer, "timeout timer", TIMEOUT_US);
            g_node.slave_check_vector = 0;
        }
    }
}