#include "mesh_transport.h"

#define TAG "MESH_TRANSPORT"

void mesh_publish_leader_election() {
    esp_err_t err = esp_ble_mesh_model_publish(&vnd_models[0], ESP_BLE_MESH_VND_MODEL_OP_LEADER_ELECTION, 0, NULL, ROLE_NODE);
    if (err) {
        ESP_LOGE(TAG, "Failed to publish leader election message (err %d)", err);
    } else {
        ESP_LOGI(TAG, "Leader election message published successfully");
    }
}

void mesh_publish_leader_victory() {
    esp_err_t err = esp_ble_mesh_model_publish(&vnd_models[0], ESP_BLE_MESH_VND_MODEL_OP_LEADER_VICTORY, 0, NULL, ROLE_NODE);
    if (err) {
        ESP_LOGE(TAG, "Failed to publish leader victory message (err %d)", err);
    } else {
        ESP_LOGI(TAG, "Leader victory message published successfully");
    }
}

void mesh_publish_heartbeat() {
    esp_err_t err = esp_ble_mesh_model_publish(&vnd_models[0], ESP_BLE_MESH_VND_MODEL_OP_HEARTBEAT, 0, NULL, ROLE_NODE);
    if (err) {
        ESP_LOGE(TAG, "Failed to publish heartbeat message (err %d)", err);
    } else {
        ESP_LOGI(TAG, "Heartbeat message published successfully");
    }
}

void mesh_publish__scene_command(scene_command_t* command) {
    esp_err_t err = esp_ble_mesh_model_publish(&vnd_models[0], ESP_BLE_MESH_VND_MODEL_OP_SCENE_COMMAND, sizeof(scene_command_t), (uint8_t*)command, ROLE_NODE);
    if (err) {
        ESP_LOGE(TAG, "Failed to publish scene command message (err %d)", err);
    } else {
        ESP_LOGI(TAG, "Scene command message published successfully");
    }
}

void mesh_publish_to_group_scene_command(uint16_t net_idx, uint16_t app_idx, uint16_t group_addr, scene_command_t* command) {
    esp_ble_mesh_msg_ctx_t ctx = {
        .net_idx = net_idx,
        .app_idx = app_idx,
        .addr = group_addr,
    };

    esp_err_t err = esp_ble_mesh_server_model_send_msg(&vnd_models[0], &ctx, ESP_BLE_MESH_VND_MODEL_OP_SCENE_COMMAND, sizeof(scene_command_t), (uint8_t*)command);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send scene command message (err %d)", err);
    } else {
        ESP_LOGI(TAG, "Scene command message sent successfully");
    }
}

void mesh_send_leader_alive(uint16_t net_idx, uint16_t app_idx, uint16_t addr) {
    esp_ble_mesh_msg_ctx_t ctx = {
        .net_idx = net_idx,
        .app_idx = app_idx,
        .addr = addr,
    };
    esp_err_t err = esp_ble_mesh_server_model_send_msg(&vnd_models[0], &ctx, ESP_BLE_MESH_VND_MODEL_OP_LEADER_ALIVE, 0, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send leader alive message (err %d)", err);
    } else {
        ESP_LOGI(TAG, "Leader alive message sent successfully");
    }
}

void mesh_send_leader_victory(uint16_t net_idx, uint16_t app_idx, uint16_t addr) {
    esp_ble_mesh_msg_ctx_t ctx = {
        .net_idx = net_idx,
        .app_idx = app_idx,
        .addr = addr,
    };
    esp_err_t err = esp_ble_mesh_server_model_send_msg(&vnd_models[0], &ctx, ESP_BLE_MESH_VND_MODEL_OP_LEADER_VICTORY, 0, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send leader victory message (err %d)", err);
    } else {
        ESP_LOGI(TAG, "Leader victory message sent successfully");
    }
}

void mesh_send_leader_ack(uint16_t net_idx, uint16_t app_idx, uint16_t addr) {
    esp_ble_mesh_msg_ctx_t ctx = {
        .net_idx = net_idx,
        .app_idx = app_idx,
        .addr = addr,
    };
    esp_err_t err = esp_ble_mesh_server_model_send_msg(&vnd_models[0], &ctx, ESP_BLE_MESH_VND_MODEL_OP_LEADER_ACK, 0, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send leader ACK message (err %d)", err);
    } else {
        ESP_LOGI(TAG, "Leader ACK message sent successfully");
    }
}

void mesh_send_heartbeat_ack(uint16_t net_idx, uint16_t app_idx, uint16_t addr) {
    esp_ble_mesh_msg_ctx_t ctx = {
        .net_idx = net_idx,
        .app_idx = app_idx,
        .addr = addr,
    };
    esp_err_t err = esp_ble_mesh_server_model_send_msg(&vnd_models[0], &ctx, ESP_BLE_MESH_VND_MODEL_OP_HEARTBEAT_ACK, 0, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send heartbeat ACK message (err %d)", err);
    } else {
        ESP_LOGI(TAG, "Heartbeat ACK message sent successfully");
    }
}

void mesh_send_sync_request(uint16_t net_idx, uint16_t app_idx, uint16_t addr, time_sync_request_t* data) {
    esp_ble_mesh_msg_ctx_t ctx = {
        .net_idx = net_idx,
        .app_idx = app_idx,
        .addr = addr,
    };
    esp_err_t err = esp_ble_mesh_server_model_send_msg(&vnd_models[0], &ctx, ESP_BLE_MESH_VND_MODEL_OP_SYNC_REQUEST, sizeof(time_sync_request_t), (uint8_t*)data);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send sync request message (err %d)", err);
    } else {
        ESP_LOGI(TAG, "Sync request message sent successfully");
    }
}

void mesh_send_sync_response(uint16_t net_idx, uint16_t app_idx, uint16_t addr, time_sync_response_t* data) {
    esp_ble_mesh_msg_ctx_t ctx = {
        .net_idx = net_idx,
        .app_idx = app_idx,
        .addr = addr,
    };
    esp_err_t err = esp_ble_mesh_server_model_send_msg(&vnd_models[0], &ctx, ESP_BLE_MESH_VND_MODEL_OP_SYNC_RESPONSE, sizeof(time_sync_response_t), (uint8_t*)data);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send sync response message (err %d)", err);
    } else {
        ESP_LOGI(TAG, "Sync response message sent successfully");
    }
}

void mesh_publish_init() { vnd_models[0].pub->publish_addr = ESP_BLE_MESH_ADDR_ALL_NODES; }