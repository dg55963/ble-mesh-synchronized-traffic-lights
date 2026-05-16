#ifndef BLE_MESH_CONFIG_H
#define BLE_MESH_CONFIG_H

#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_defs.h"

#define CID_ESP 0xFFFF

#define ESP_BLE_MESH_VND_MODEL_ID_SERVER 0x0000

#define ESP_BLE_MESH_VND_MODEL_OP_LEADER_ELECTION ESP_BLE_MESH_MODEL_OP_3(0x00, CID_ESP)
#define ESP_BLE_MESH_VND_MODEL_OP_LEADER_ALIVE ESP_BLE_MESH_MODEL_OP_3(0x01, CID_ESP)
#define ESP_BLE_MESH_VND_MODEL_OP_LEADER_VICTORY ESP_BLE_MESH_MODEL_OP_3(0x02, CID_ESP)
#define ESP_BLE_MESH_VND_MODEL_OP_LEADER_ACK ESP_BLE_MESH_MODEL_OP_3(0x03, CID_ESP)
#define ESP_BLE_MESH_VND_MODEL_OP_HEARTBEAT ESP_BLE_MESH_MODEL_OP_3(0x04, CID_ESP)
#define ESP_BLE_MESH_VND_MODEL_OP_HEARTBEAT_ACK ESP_BLE_MESH_MODEL_OP_3(0x05, CID_ESP)
#define ESP_BLE_MESH_VND_MODEL_OP_HEARTBEAT_TIMEOUT ESP_BLE_MESH_MODEL_OP_3(0x06, CID_ESP)

extern uint8_t dev_uuid[16];
extern esp_ble_mesh_prov_t provision;
extern esp_ble_mesh_comp_t composition;
extern esp_ble_mesh_model_t vnd_models[];
extern esp_ble_mesh_elem_t elements[];

#endif  // BLE_MESH_CONFIG_H