#ifndef TRAFFIC_CONTROLLER_H
#define TRAFFIC_CONTROLLER_H

#include <stdint.h>
#include <stdlib.h>

#include "ble_mesh_config.h"
#include "board.h"
#include "mesh_transport.h"
#include "my_timers.h"
#include "traffic_types.h"

#define TRAFFIC_CONTROLLER_TAG "TRAFFIC_CTRL"

#define START_DELAY_MS 500
#define RED_DURATION_MS 1000
#define RED_YELLOW_DURATION_MS 2000
#define YELLOW_DURATION_MS 3000
#define BLINK_INTERVAL_MS 500

#define TRAFFIC_GROUP1_ADDR 0xC000
#define TRAFFIC_GROUP2_ADDR 0xC001

extern traffic_state_t g_current_traffic_state;

void traffic_apply_scene(traffic_scene_t scene);
void scheduler_step(bool blink);
void start_scheduler();
#endif  // TRAFFIC_CONTROLLER_H