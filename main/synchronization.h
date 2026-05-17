#ifndef SYNCHRONIZATION_H
#define SYNCHRONIZATION_H

#define SYNC_TAG "SYNC"

#include <stdint.h>

#include "esp_log.h"
#include "esp_timer.h"
#include "node_state.h"

typedef struct {
    uint8_t request_id;
    uint32_t t0;
    int32_t clock_offset_ms;
} time_sync_state_t;

typedef struct __attribute__((packed)) {
    uint32_t request_id;
} time_sync_request_t;

typedef struct __attribute__((packed)) {
    uint32_t request_id;
    uint32_t leader_time_ms;
} time_sync_response_t;

#include "mesh_transport.h"

extern time_sync_state_t g_sync;

void send_sync_request();

#endif  // SYNCHRONIZATION_H