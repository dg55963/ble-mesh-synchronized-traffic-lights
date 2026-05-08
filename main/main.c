/* main.c - Application main entry point */

/*
 * SPDX-FileCopyrightText: 2017 Intel Corporation
 * SPDX-FileContributor: 2018-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

// ESP-IDF includes
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs_flash.h"

// BLE Mesh includes
#include "ble_mesh_example_init.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_local_data_operation_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_provisioning_api.h"

// my includes
#include "ble_mesh_config.h"
#include "board.h"

#define TAG "MAIN"

#define LEADER_ELECTION_TIMEOUT_US 2000000

uint16_t my_addr = BLE_MESH_ADDR_UNASSIGNED;
uint16_t net_idx = BLE_MESH_KEY_UNUSED;

extern struct _led_state led_state[3];

bool i_am_leader = false;
bool leader_elected = false;
bool leader_election_started = false;
uint16_t leader_addr = BLE_MESH_ADDR_UNASSIGNED;
void leader_timer_cb(void* arg);
esp_timer_handle_t leader_timer;
esp_timer_create_args_t leader_timer_args = {
    .callback = leader_timer_cb,
    .name = "leader_timer",
};

void start_leader_election() {
    ESP_LOGI(TAG, "Starting leader election...");

    esp_err_t err = esp_ble_mesh_model_publish(&vnd_models[0], ESP_BLE_MESH_VND_MODEL_OP_LEADER_ELECTION, 0, NULL, ROLE_NODE);
    if (err) {
        ESP_LOGE(TAG, "Failed to publish leader election message (err %d)", err);
    } else {
        ESP_LOGI(TAG, "Leader election message published successfully");
    }

    if (esp_timer_is_active(leader_timer)) {
        esp_timer_stop(leader_timer);
    }
    esp_timer_start_once(leader_timer, LEADER_ELECTION_TIMEOUT_US);
}

bool check_and_set_app_key_before_election() {
    if (esp_ble_mesh_node_is_provisioned()) {
        uint16_t app_idx = elements[0].vnd_models[0].keys[0];

        if (esp_ble_mesh_node_get_local_app_key(app_idx) != NULL) {
            vnd_models[0].pub->app_idx = app_idx;
            vnd_models[0].pub->publish_addr = ESP_BLE_MESH_ADDR_ALL_NODES;
            vTaskDelay(pdMS_TO_TICKS(1000));
            return true;
        } else {
            ESP_LOGW(TAG, "No AppKey bound to the vendor model, cannot start leader election");
        }
    } else {
        ESP_LOGI(TAG, "Node is not provisioned, waiting for provisioning to complete...");
    }
    return false;
}

void leader_timer_cb(void* arg) {
    esp_err_t err = esp_ble_mesh_model_publish(&vnd_models[0], ESP_BLE_MESH_VND_MODEL_OP_LEADER_VICTORY, 0, NULL, ROLE_NODE);
    if (err) {
        ESP_LOGE(TAG, "Failed to publish leader victory message (err %d)", err);
    } else {
        ESP_LOGI(TAG, "Leader victory message published successfully");
    }
    i_am_leader = true;
    leader_elected = true;
    leader_election_started = false;
    leader_addr = my_addr;
    if (esp_timer_is_active(leader_timer)) {
        esp_timer_stop(leader_timer);
    }
}

static void prov_complete(uint16_t _net_idx, uint16_t _addr, uint8_t flags, uint32_t iv_index) {
    ESP_LOGI(TAG, "net_idx: 0x%04x, addr: 0x%04x", _net_idx, _addr);
    ESP_LOGI(TAG, "flags: 0x%02x, iv_index: 0x%08" PRIx32, flags, iv_index);
    net_idx = _net_idx;
    my_addr = _addr;
}

static void ble_mesh_provisioning_cb(esp_ble_mesh_prov_cb_event_t event, esp_ble_mesh_prov_cb_param_t* param) {
    switch (event) {
        case ESP_BLE_MESH_PROV_REGISTER_COMP_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_PROV_REGISTER_COMP_EVT, err_code %d", param->prov_register_comp.err_code);
            break;
        case ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT, err_code %d", param->node_prov_enable_comp.err_code);
            break;
        case ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT, bearer %s", param->node_prov_link_open.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
            break;
        case ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT, bearer %s", param->node_prov_link_close.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
            break;
        case ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT");
            prov_complete(param->node_prov_complete.net_idx, param->node_prov_complete.addr, param->node_prov_complete.flags, param->node_prov_complete.iv_index);
            break;
        case ESP_BLE_MESH_NODE_PROV_RESET_EVT:
            break;
        case ESP_BLE_MESH_NODE_SET_UNPROV_DEV_NAME_COMP_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_SET_UNPROV_DEV_NAME_COMP_EVT, err_code %d", param->node_set_unprov_dev_name_comp.err_code);
            break;
        default:
            break;
    }
}

static void ble_mesh_config_server_cb(esp_ble_mesh_cfg_server_cb_event_t event, esp_ble_mesh_cfg_server_cb_param_t* param) {
    if (event == ESP_BLE_MESH_CFG_SERVER_STATE_CHANGE_EVT) {
        switch (param->ctx.recv_op) {
            case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:
                ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD");
                ESP_LOGI(TAG, "net_idx 0x%04x, app_idx 0x%04x", param->value.state_change.appkey_add.net_idx, param->value.state_change.appkey_add.app_idx);
                ESP_LOG_BUFFER_HEX("AppKey", param->value.state_change.appkey_add.app_key, 16);
                break;
            case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND:
                ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND");
                ESP_LOGI(TAG, "elem_addr 0x%04x, app_idx 0x%04x, cid 0x%04x, mod_id 0x%04x", param->value.state_change.mod_app_bind.element_addr, param->value.state_change.mod_app_bind.app_idx,
                         param->value.state_change.mod_app_bind.company_id, param->value.state_change.mod_app_bind.model_id);
                break;
            case ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD:
                ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD");
                ESP_LOGI(TAG, "elem_addr 0x%04x, sub_addr 0x%04x, cid 0x%04x, mod_id 0x%04x", param->value.state_change.mod_sub_add.element_addr, param->value.state_change.mod_sub_add.sub_addr,
                         param->value.state_change.mod_sub_add.company_id, param->value.state_change.mod_sub_add.model_id);
                break;
            default:
                break;
        }
    }
}

static void ble_mesh_custom_model_cb(esp_ble_mesh_model_cb_event_t event, esp_ble_mesh_model_cb_param_t* param) {
    if (event == ESP_BLE_MESH_MODEL_OPERATION_EVT) {
        if (param->model_operation.ctx->addr != my_addr) {
            switch (param->model_operation.opcode) {
                case ESP_BLE_MESH_VND_MODEL_OP_LEADER_ELECTION:
                    ESP_LOGI(TAG, "Leader election message received from addr 0x%04x", param->model_operation.ctx->addr);
                    if (param->model_operation.ctx->addr > my_addr) {
                        esp_ble_mesh_msg_ctx_t ctx = {
                            .net_idx = net_idx,
                            .app_idx = param->model_operation.ctx->app_idx,
                            .addr = param->model_operation.ctx->addr,
                        };
                        esp_err_t err = esp_ble_mesh_server_model_send_msg(param->model_operation.model, &ctx, ESP_BLE_MESH_VND_MODEL_OP_LEADER_ALIVE, 0, NULL);
                        if (err != ESP_OK) {
                            ESP_LOGE(TAG, "Failed to send leader alive message (err %d)", err);
                        } else {
                            ESP_LOGI(TAG, "Leader alive message sent successfully");
                        }
                        if (!leader_election_started) {
                            start_leader_election();
                            leader_election_started = true;
                        }
                    }
                    break;
                case ESP_BLE_MESH_VND_MODEL_OP_LEADER_ALIVE:
                    ESP_LOGI(TAG, "Leader alive message received from addr 0x%04x", param->model_operation.ctx->addr);
                    i_am_leader = false;
                    esp_timer_stop(leader_timer);
                    break;
                case ESP_BLE_MESH_VND_MODEL_OP_LEADER_VICTORY:
                    ESP_LOGI(TAG, "Leader victory message received from addr 0x%04x", param->model_operation.ctx->addr);
                    if (param->model_operation.ctx->addr > my_addr) {
                        esp_ble_mesh_msg_ctx_t ctx = {
                            .net_idx = net_idx,
                            .app_idx = param->model_operation.ctx->app_idx,
                            .addr = param->model_operation.ctx->addr,
                        };
                        esp_err_t err = esp_ble_mesh_server_model_send_msg(param->model_operation.model, &ctx, ESP_BLE_MESH_VND_MODEL_OP_LEADER_ALIVE, 0, NULL);
                        if (err != ESP_OK) {
                            ESP_LOGE(TAG, "Failed to send leader alive message (err %d)", err);
                        } else {
                            ESP_LOGI(TAG, "Leader alive message sent successfully");
                        }
                        start_leader_election();
                        leader_election_started = true;
                    } else {
                        i_am_leader = false;
                        leader_elected = true;
                        leader_election_started = false;
                        leader_addr = param->model_operation.ctx->addr;
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

static esp_err_t ble_mesh_init(void) {
    esp_err_t err = ESP_OK;

    esp_ble_mesh_register_prov_callback(ble_mesh_provisioning_cb);
    esp_ble_mesh_register_config_server_callback(ble_mesh_config_server_cb);
    esp_ble_mesh_register_custom_model_callback(ble_mesh_custom_model_cb);

    err = esp_ble_mesh_init(&provision, &composition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize mesh stack (err %d)", err);
        return err;
    }

    if (!esp_ble_mesh_node_is_provisioned()) {
        err = esp_ble_mesh_node_prov_enable((esp_ble_mesh_prov_bearer_t)(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT));
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to enable mesh node (err %d)", err);
            return err;
        }
    }

    ESP_LOGI(TAG, "BLE Mesh Node initialized");

    return err;
}

void app_main(void) {
    esp_err_t err;

    ESP_LOGI(TAG, "Initializing...");

    board_init();

    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    err = bluetooth_init();
    if (err) {
        ESP_LOGE(TAG, "esp32_bluetooth_init failed (err %d)", err);
        return;
    }

    ble_mesh_get_dev_uuid(dev_uuid);

    /* Initialize the Bluetooth Mesh Subsystem */
    err = ble_mesh_init();
    if (err) {
        ESP_LOGE(TAG, "Bluetooth mesh init failed (err %d)", err);
    }

    vTaskDelay(pdMS_TO_TICKS(1000));

    while (1) {
        if (leader_elected) {
            if (i_am_leader) {
                ESP_LOGI(TAG, "I am the leader (addr: 0x%04x)", my_addr);
                board_led_operation(LED_RED, LED_ON);
                board_led_operation(LED_GREEN, LED_OFF);
            } else {
                ESP_LOGI(TAG, "Leader elected (addr: 0x%04x)", leader_addr);
                board_led_operation(LED_RED, LED_OFF);
                board_led_operation(LED_GREEN, LED_ON);
            }
            vTaskDelay(pdMS_TO_TICKS(5000));
        } else {
            if (check_and_set_app_key_before_election() && !leader_election_started) {
                esp_timer_create(&leader_timer_args, &leader_timer);
                start_leader_election();
                leader_election_started = true;
            }
        }
    }
}
