#include "synchronization.h"

time_sync_state_t g_sync = {
    .request_id = 0,
    .t0 = 0,
    .clock_offset_ms = 0,
};

void send_sync_request() {
    g_sync.t0 = esp_timer_get_time() / 1000;
    g_sync.request_id++;

    time_sync_request_t request = {
        .request_id = g_sync.request_id,
    };
    mesh_send_sync_request(g_node.net_idx, vnd_models[0].keys[0], g_node.leader_addr, &request);

    ESP_LOGI(SYNC_TAG, "Sync request sent (id=%u)", request.request_id);
}