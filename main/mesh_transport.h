#ifndef MESH_TRANSPORT_H
#define MESH_TRANSPORT_H

#include "ble_mesh_config.h"
#include "esp_ble_mesh_networking_api.h"
#include "synchronization.h"
#include "traffic_types.h"

void mesh_publish_leader_election();
void mesh_publish_leader_victory();
void mesh_publish_heartbeat();
void mesh_publish__scene_command(scene_command_t* command);
void mesh_publish_to_group_scene_command(uint16_t net_idx, uint16_t app_idx, uint16_t group_addr, scene_command_t* command);

void mesh_send_leader_alive(uint16_t net_idx, uint16_t app_idx, uint16_t addr);
void mesh_send_leader_victory(uint16_t net_idx, uint16_t app_idx, uint16_t addr);
void mesh_send_leader_ack(uint16_t net_idx, uint16_t app_idx, uint16_t addr);
void mesh_send_heartbeat_ack(uint16_t net_idx, uint16_t app_idx, uint16_t addr);
void mesh_send_sync_request(uint16_t net_idx, uint16_t app_idx, uint16_t addr, time_sync_request_t* data);
void mesh_send_sync_response(uint16_t net_idx, uint16_t app_idx, uint16_t addr, time_sync_response_t* data);

void mesh_publish_init();

#endif  // MESH_TRANSPORT_H