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
        stop_timer(sync_timer);
        stop_timer(traffic_control_timer);
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
        start_sync();
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
        if (g_node.slave_count > 0 && g_node.slave_check_vector == ((1 << (NUM_OF_NODES - 1)) - 1)) {
            ESP_LOGI(MESH_HANDLERS_TAG, "Received heartbeat ACK from all slaves");
            if (g_current_traffic_state.blinking) {
                g_current_traffic_state.blinking = false;
                traffic_reset_phase();
            }
            start_timer_once(timeout_timer, "timeout timer", TIMEOUT_US);
            g_node.slave_check_vector = 0;
        }
    }
}

void handle_sync_request(esp_ble_mesh_model_cb_param_t* param) {
    time_sync_request_t* request = (time_sync_request_t*)param->model_operation.msg;

    time_sync_response_t response = {
        .request_id = request->request_id,
        .leader_time_ms = esp_timer_get_time() / 1000,
    };

    mesh_send_sync_response(param->model_operation.ctx->net_idx, param->model_operation.ctx->app_idx, param->model_operation.ctx->addr, &response);
}

void handle_sync_response(esp_ble_mesh_model_cb_param_t* param) {
    time_sync_response_t* response = (time_sync_response_t*)param->model_operation.msg;

    if (response->request_id != g_sync.request_id) {
        return;
    }

    uint64_t t1 = esp_timer_get_time() / 1000;

    int64_t rtt = t1 - g_sync.t0;  // Round-trip time

    int64_t estimated_delay = rtt / 2;

    int64_t estimated_leader_now = response->leader_time_ms + estimated_delay;

    g_sync.clock_offset_ms = estimated_leader_now - t1;

    ESP_LOGI(MESH_HANDLERS_TAG, "Clock offset updated: %ld ms", g_sync.clock_offset_ms);
}

void handle_scene_command(esp_ble_mesh_model_cb_param_t* param) {
    scene_command_t* command = (scene_command_t*)param->model_operation.msg;
    int64_t start_in_ms = command->execute_at_ms - (esp_timer_get_time() / 1000 + g_sync.clock_offset_ms);
    g_current_traffic_state.scene = command->scene;
    if (start_in_ms < 0) {
        traffic_apply_scene(command->scene);
        return;
    } else {
        start_timer_once(scene_timer, "scene timer", start_in_ms * 1000);
    }
}