#ifndef MY_TIMERS_H
#define MY_TIMERS_H

#define TIMER_TAG "TIMERS"
#define LEADER_ELECTION_TIMEOUT_US 2000000
#define HEARTBEAT_INTERVAL_US 2000000
#define TIMEOUT_US 5000000
#define SYNC_INTERVAL_US 5000000

#include "esp_log.h"
#include "esp_timer.h"
#include "mesh_transport.h"
#include "node_logic.h"
#include "node_state.h"
#include "traffic_controller.h"

void leader_victory_timer_cb(void* arg);
void heartbeat_timer_cb(void* arg);
void timeout_timer_cb(void* arg);
void sync_timer_cb(void* arg);
void scene_timer_cb(void* arg);
void traffic_control_timer_cb(void* arg);

extern esp_timer_handle_t leader_victory_timer;
extern esp_timer_create_args_t leader_victory_timer_args;

extern esp_timer_handle_t heartbeat_timer;
extern esp_timer_create_args_t heartbeat_timer_args;

extern esp_timer_handle_t timeout_timer;
extern esp_timer_create_args_t timeout_timer_args;

extern esp_timer_handle_t sync_timer;
extern esp_timer_create_args_t sync_timer_args;

extern esp_timer_handle_t scene_timer;
extern esp_timer_create_args_t scene_timer_args;

extern esp_timer_handle_t traffic_control_timer;
extern esp_timer_create_args_t traffic_control_timer_args;

void start_timer_once(esp_timer_handle_t timer, const char* name, uint64_t timeout_us);
void start_timer_periodic(esp_timer_handle_t timer, const char* name, uint64_t interval_us);

void stop_timer(esp_timer_handle_t timer);

void timers_init();

#endif  // MY_TIMERS_H
