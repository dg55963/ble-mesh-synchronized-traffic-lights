#ifndef MESH_HANDLERS_H
#define MESH_HANDLERS_H

#define MESH_HANDLERS_TAG "MESH_HANDLERS"

#include "esp_log.h"
#include "mesh_transport.h"
#include "my_timers.h"
#include "node_state.h"

void handle_leader_election(esp_ble_mesh_model_cb_param_t* param);
void handle_leader_alive(esp_ble_mesh_model_cb_param_t* param);
void handle_leader_victory(esp_ble_mesh_model_cb_param_t* param);
void handle_leader_ack(esp_ble_mesh_model_cb_param_t* param);
void handle_heartbeat(esp_ble_mesh_model_cb_param_t* param);
void handle_heartbeat_ack(esp_ble_mesh_model_cb_param_t* param);

#endif  // MESH_HANDLERS_H