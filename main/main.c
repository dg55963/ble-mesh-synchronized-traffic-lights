#include <inttypes.h>

// ESP-IDF includes
#include "esp_log.h"
#include "nvs_flash.h"

// BLE Mesh includes
#include "ble_mesh_example_init.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_local_data_operation_api.h"
#include "esp_ble_mesh_provisioning_api.h"

// my includes
#include "ble_mesh_config.h"
#include "board.h"
#include "mesh_handlers.h"
#include "my_timers.h"
#include "node_logic.h"
#include "node_state.h"
#include "traffic_controller.h"

#define MAIN_TAG "MAIN"

static void prov_complete(uint16_t _net_idx, uint16_t _addr, uint8_t flags, uint32_t iv_index) {
    ESP_LOGI(MAIN_TAG, "net_idx: 0x%04x, addr: 0x%04x", _net_idx, _addr);
    ESP_LOGI(MAIN_TAG, "flags: 0x%02x, iv_index: 0x%08" PRIx32, flags, iv_index);
    g_node.my_addr = _addr;
    g_node.net_idx = _net_idx;
}

static void ble_mesh_provisioning_cb(esp_ble_mesh_prov_cb_event_t event, esp_ble_mesh_prov_cb_param_t* param) {
    switch (event) {
        case ESP_BLE_MESH_PROV_REGISTER_COMP_EVT:
            ESP_LOGI(MAIN_TAG, "ESP_BLE_MESH_PROV_REGISTER_COMP_EVT, err_code %d", param->prov_register_comp.err_code);
            break;
        case ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT:
            ESP_LOGI(MAIN_TAG, "ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT, err_code %d", param->node_prov_enable_comp.err_code);
            break;
        case ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT:
            ESP_LOGI(MAIN_TAG, "ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT, bearer %s", param->node_prov_link_open.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
            break;
        case ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT:
            ESP_LOGI(MAIN_TAG, "ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT, bearer %s", param->node_prov_link_close.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
            break;
        case ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT:
            ESP_LOGI(MAIN_TAG, "ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT");
            prov_complete(param->node_prov_complete.net_idx, param->node_prov_complete.addr, param->node_prov_complete.flags, param->node_prov_complete.iv_index);
            break;
        case ESP_BLE_MESH_NODE_PROV_RESET_EVT:
            break;
        case ESP_BLE_MESH_NODE_SET_UNPROV_DEV_NAME_COMP_EVT:
            ESP_LOGI(MAIN_TAG, "ESP_BLE_MESH_NODE_SET_UNPROV_DEV_NAME_COMP_EVT, err_code %d", param->node_set_unprov_dev_name_comp.err_code);
            break;
        default:
            break;
    }
}

static void ble_mesh_config_server_cb(esp_ble_mesh_cfg_server_cb_event_t event, esp_ble_mesh_cfg_server_cb_param_t* param) {
    if (event == ESP_BLE_MESH_CFG_SERVER_STATE_CHANGE_EVT) {
        switch (param->ctx.recv_op) {
            case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:
                ESP_LOGI(MAIN_TAG, "ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD");
                ESP_LOGI(MAIN_TAG, "net_idx 0x%04x, app_idx 0x%04x", param->value.state_change.appkey_add.net_idx, param->value.state_change.appkey_add.app_idx);
                ESP_LOG_BUFFER_HEX("AppKey", param->value.state_change.appkey_add.app_key, 16);
                break;
            case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND:
                ESP_LOGI(MAIN_TAG, "ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND");
                ESP_LOGI(MAIN_TAG, "elem_addr 0x%04x, app_idx 0x%04x, cid 0x%04x, mod_id 0x%04x", param->value.state_change.mod_app_bind.element_addr, param->value.state_change.mod_app_bind.app_idx,
                         param->value.state_change.mod_app_bind.company_id, param->value.state_change.mod_app_bind.model_id);
                break;
            case ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD:
                ESP_LOGI(MAIN_TAG, "ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD");
                ESP_LOGI(MAIN_TAG, "elem_addr 0x%04x, sub_addr 0x%04x, cid 0x%04x, mod_id 0x%04x", param->value.state_change.mod_sub_add.element_addr, param->value.state_change.mod_sub_add.sub_addr,
                         param->value.state_change.mod_sub_add.company_id, param->value.state_change.mod_sub_add.model_id);
                break;
            default:
                break;
        }
    }
}

static void ble_mesh_custom_model_cb(esp_ble_mesh_model_cb_event_t event, esp_ble_mesh_model_cb_param_t* param) {
    if (event == ESP_BLE_MESH_MODEL_OPERATION_EVT) {
        if (param->model_operation.ctx->addr != g_node.my_addr) {
            switch (param->model_operation.opcode) {
                case ESP_BLE_MESH_VND_MODEL_OP_LEADER_ELECTION:
                    ESP_LOGI(MAIN_TAG, "Leader election message received from addr 0x%04x", param->model_operation.ctx->addr);
                    handle_leader_election(param);
                    return;

                case ESP_BLE_MESH_VND_MODEL_OP_LEADER_ALIVE:
                    ESP_LOGI(MAIN_TAG, "Leader alive message received from addr 0x%04x", param->model_operation.ctx->addr);
                    handle_leader_alive(param);
                    return;

                case ESP_BLE_MESH_VND_MODEL_OP_LEADER_VICTORY:
                    ESP_LOGI(MAIN_TAG, "Leader victory message received from addr 0x%04x", param->model_operation.ctx->addr);
                    handle_leader_victory(param);
                    break;

                case ESP_BLE_MESH_VND_MODEL_OP_LEADER_ACK:
                    ESP_LOGI(MAIN_TAG, "Leader ACK message received from addr 0x%04x", param->model_operation.ctx->addr);
                    handle_leader_ack(param);
                    break;

                case ESP_BLE_MESH_VND_MODEL_OP_HEARTBEAT:
                    ESP_LOGI(MAIN_TAG, "Heartbeat message received from addr 0x%04x", param->model_operation.ctx->addr);
                    handle_heartbeat(param);
                    break;

                case ESP_BLE_MESH_VND_MODEL_OP_HEARTBEAT_ACK:
                    handle_heartbeat_ack(param);
                    break;

                case ESP_BLE_MESH_VND_MODEL_OP_SYNC_REQUEST:
                    ESP_LOGI(MAIN_TAG, "Sync request message received from addr 0x%04x", param->model_operation.ctx->addr);
                    handle_sync_request(param);
                    break;

                case ESP_BLE_MESH_VND_MODEL_OP_SYNC_RESPONSE:
                    ESP_LOGI(MAIN_TAG, "Sync response message received from addr 0x%04x", param->model_operation.ctx->addr);
                    handle_sync_response(param);
                    break;

                case ESP_BLE_MESH_VND_MODEL_OP_SCENE_COMMAND:
                    handle_scene_command(param);
                    break;

                default:
                    break;
            }
        } else {
            switch (param->model_operation.opcode) {
                case ESP_BLE_MESH_VND_MODEL_OP_SYNC_REQUEST:
                    ESP_LOGI(MAIN_TAG, "Sync request message received from myself");
                    handle_sync_request(param);
                    break;

                case ESP_BLE_MESH_VND_MODEL_OP_SYNC_RESPONSE:
                    ESP_LOGI(MAIN_TAG, "Sync response message received from myself");
                    handle_sync_response(param);
                    break;

                case ESP_BLE_MESH_VND_MODEL_OP_SCENE_COMMAND:
                    handle_scene_command(param);
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
        ESP_LOGE(MAIN_TAG, "Failed to initialize mesh stack (err %d)", err);
        return err;
    }

    if (!esp_ble_mesh_node_is_provisioned()) {
        err = esp_ble_mesh_node_prov_enable((esp_ble_mesh_prov_bearer_t)(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT));
        if (err != ESP_OK) {
            ESP_LOGE(MAIN_TAG, "Failed to enable mesh node (err %d)", err);
            return err;
        }
    }

    ESP_LOGI(MAIN_TAG, "BLE Mesh Node initialized");

    return err;
}

void app_main(void) {
    esp_err_t err;

    ESP_LOGI(MAIN_TAG, "Initializing...");

    board_init();

    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    err = bluetooth_init();
    if (err) {
        ESP_LOGE(MAIN_TAG, "esp32_bluetooth_init failed (err %d)", err);
        return;
    }

    ble_mesh_get_dev_uuid(dev_uuid);

    /* Initialize the Bluetooth Mesh Subsystem */
    err = ble_mesh_init();
    if (err) {
        ESP_LOGE(MAIN_TAG, "Bluetooth mesh init failed (err %d)", err);
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
    timers_init();
    mesh_publish_init();

    while (!esp_ble_mesh_node_is_provisioned()) {
        ESP_LOGI(MAIN_TAG, "Waiting for provisioning...");
        vTaskDelay(pdMS_TO_TICKS(3000));
    }

    start_leader_election();
    while (node_is_idle()) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
