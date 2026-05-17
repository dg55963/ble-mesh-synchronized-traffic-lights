#ifndef TRAFFIC_TYPES_H
#define TRAFFIC_TYPES_H

#include <inttypes.h>

typedef enum {
    TRAFFIC_SCENE_ONLY_RED,
    TRAFFIC_SCENE_RED_AND_YELLOW,
    TRAFFIC_SCENE_ONLY_GREEN,
    TRAFFIC_SCENE_ONLY_YELLOW,
    TRAFFIC_SCENE_ALL_OFF,
} traffic_scene_t;

typedef struct __attribute__((packed)) {
    traffic_scene_t scene;
    uint32_t execute_at_ms;
} scene_command_t;

typedef struct {
    traffic_scene_t scene;
    bool blinking;
} traffic_state_t;

#endif  // TRAFFIC_TYPES_H