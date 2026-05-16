#ifndef MESH_TRANSPORT_H
#define MESH_TRANSPORT_H

#include "ble_mesh_config.h"
#include "esp_ble_mesh_networking_api.h"

void mesh_publish_leader_election();
void mesh_publish_leader_victory();
void mesh_publish_heartbeat();

void mesh_send_leader_alive(uint16_t net_idx, uint16_t app_idx, uint16_t addr);
void mesh_send_leader_victory(uint16_t net_idx, uint16_t app_idx, uint16_t addr);
void mesh_send_leader_ack(uint16_t net_idx, uint16_t app_idx, uint16_t addr);
void mesh_send_heartbeat_ack(uint16_t net_idx, uint16_t app_idx, uint16_t addr);

void mesh_publish_init();

#endif  // MESH_TRANSPORT_H