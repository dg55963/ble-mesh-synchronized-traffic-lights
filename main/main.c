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

#include "ble_mesh_example_init.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_local_data_operation_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_log.h"
#include "nvs_flash.h"

// my includes
#include "ble_mesh_config.h"
#include "board.h"

#define TAG "EXAMPLE"

extern struct _led_state led_state[3];

static void prov_complete(uint16_t net_idx, uint16_t addr, uint8_t flags, uint32_t iv_index) {
    ESP_LOGI(TAG, "net_idx: 0x%04x, addr: 0x%04x", net_idx, addr);
    ESP_LOGI(TAG, "flags: 0x%02x, iv_index: 0x%08" PRIx32, flags, iv_index);
    board_led_operation(LED_RED, LED_OFF);
}

static void example_change_led_state(esp_ble_mesh_model_t* model, esp_ble_mesh_msg_ctx_t* ctx, uint8_t onoff) {
    uint16_t primary_addr = esp_ble_mesh_get_primary_element_address();
    uint8_t elem_count = esp_ble_mesh_get_element_count();
    struct _led_state* led = NULL;
    uint8_t i;

    if (ESP_BLE_MESH_ADDR_IS_UNICAST(ctx->recv_dst)) {
        for (i = 0; i < elem_count; i++) {
            if (ctx->recv_dst == (primary_addr + i)) {
                led = &led_state[i];
                board_led_operation(led->pin, onoff);
            }
        }
    } else if (ESP_BLE_MESH_ADDR_IS_GROUP(ctx->recv_dst)) {
        if (esp_ble_mesh_is_model_subscribed_to_group(model, ctx->recv_dst)) {
            led = &led_state[model->element->element_addr - primary_addr];
            board_led_operation(led->pin, onoff);
        }
    } else if (ctx->recv_dst == 0xFFFF) {
        led = &led_state[model->element->element_addr - primary_addr];
        board_led_operation(led->pin, onoff);
    }
}

static void example_ble_mesh_provisioning_cb(esp_ble_mesh_prov_cb_event_t event, esp_ble_mesh_prov_cb_param_t* param) {
    switch (event) {
        case ESP_BLE_MESH_PROV_REGISTER_COMP_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_PROV_REGISTER_COMP_EVT, err_code %d", param->prov_register_comp.err_code);
            if (esp_ble_mesh_node_is_provisioned()) {
                ESP_LOGI(TAG, "Node already provisioned");
                board_led_operation(LED_RED, LED_OFF);
            } else {
                ESP_LOGI(TAG, "Node NOT provisioned");
                board_led_operation(LED_RED, LED_ON);
            }
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

static void example_ble_mesh_config_server_cb(esp_ble_mesh_cfg_server_cb_event_t event, esp_ble_mesh_cfg_server_cb_param_t* param) {
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

static esp_err_t ble_mesh_init(void) {
    esp_err_t err = ESP_OK;

    esp_ble_mesh_register_prov_callback(example_ble_mesh_provisioning_cb);
    esp_ble_mesh_register_config_server_callback(example_ble_mesh_config_server_cb);

    err = esp_ble_mesh_init(&provision, &composition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize mesh stack (err %d)", err);
        return err;
    }

    err = esp_ble_mesh_node_prov_enable((esp_ble_mesh_prov_bearer_t)(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable mesh node (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "BLE Mesh Node initialized");

    board_led_operation(LED_RED, LED_ON);

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
}
