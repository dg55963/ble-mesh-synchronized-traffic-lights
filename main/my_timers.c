#include "my_timers.h"

esp_timer_handle_t leader_victory_timer;
esp_timer_create_args_t leader_victory_timer_args = {
    .callback = leader_victory_timer_cb,
    .name = "leader_victory_timer",
};

esp_timer_handle_t heartbeat_timer;
esp_timer_create_args_t heartbeat_timer_args = {
    .callback = heartbeat_timer_cb,
    .name = "heartbeat_timer",
};

esp_timer_handle_t timeout_timer;
esp_timer_create_args_t timeout_timer_args = {
    .callback = timeout_timer_cb,
    .name = "timeout_timer",
};

esp_timer_handle_t sync_timer;
esp_timer_create_args_t sync_timer_args = {
    .callback = sync_timer_cb,
    .name = "sync_timer",
};

esp_timer_handle_t scene_timer;
esp_timer_create_args_t scene_timer_args = {
    .callback = scene_timer_cb,
    .name = "scene_timer",
    .arg = &g_current_traffic_state,
};

esp_timer_handle_t traffic_control_timer;
esp_timer_create_args_t traffic_control_timer_args = {
    .callback = traffic_control_timer_cb,
    .name = "traffic_control_timer",
};

void leader_victory_timer_cb(void* arg) {
    if (node_has_leader()) return;
    node_become_leader();
    start_sync();
    mesh_publish_leader_victory();
    start_heartbeat();
}

void heartbeat_timer_cb(void* arg) { mesh_publish_heartbeat(); }

void timeout_timer_cb(void* arg) {
    if (node_is_leader()) {
        ESP_LOGW(TIMER_TAG, "I was the leader, but seems like I lost connection with slaves.");
        g_current_traffic_state.blinking = true;
        start_scheduler();
    } else {
        ESP_LOGW(TIMER_TAG, "Leader timeout, starting election...");
        node_reset();
        stop_timer(sync_timer);
        start_leader_election();
    }
}

void sync_timer_cb(void* arg) { send_sync_request(); }

void scene_timer_cb(void* arg) { traffic_apply_scene(((traffic_state_t*)arg)->scene); }

void traffic_control_timer_cb(void* arg) { scheduler_step(); }

void start_timer_once(esp_timer_handle_t timer, const char* name, uint64_t timeout_us) {
    stop_timer(timer);
    esp_err_t err = esp_timer_start_once(timer, timeout_us);
    if (err) {
        ESP_LOGE(TIMER_TAG, "Failed to start %s timer (err %d)", name, err);
    }
}

void start_timer_periodic(esp_timer_handle_t timer, const char* name, uint64_t interval_us) {
    stop_timer(timer);
    esp_err_t err = esp_timer_start_periodic(timer, interval_us);
    if (err) {
        ESP_LOGE(TIMER_TAG, "Failed to periodic start %s timer (err %d)", name, err);
    }
}

void stop_timer(esp_timer_handle_t timer) {
    if (esp_timer_is_active(timer)) {
        esp_err_t err = esp_timer_stop(timer);
        if (err) {
            ESP_LOGE(TIMER_TAG, "Failed to stop timer (err %d)", err);
        }
    }
}

void timers_init() {
    if (esp_timer_create(&leader_victory_timer_args, &leader_victory_timer) != ESP_OK) {
        ESP_LOGE(TIMER_TAG, "Failed to create leader victory timer");
    }
    if (esp_timer_create(&heartbeat_timer_args, &heartbeat_timer) != ESP_OK) {
        ESP_LOGE(TIMER_TAG, "Failed to create heartbeat timer");
    }
    if (esp_timer_create(&timeout_timer_args, &timeout_timer) != ESP_OK) {
        ESP_LOGE(TIMER_TAG, "Failed to create timeout timer");
    }
    if (esp_timer_create(&sync_timer_args, &sync_timer) != ESP_OK) {
        ESP_LOGE(TIMER_TAG, "Failed to create sync timer");
    }
    if (esp_timer_create(&scene_timer_args, &scene_timer) != ESP_OK) {
        ESP_LOGE(TIMER_TAG, "Failed to create scene timer");
    }
    if (esp_timer_create(&traffic_control_timer_args, &traffic_control_timer) != ESP_OK) {
        ESP_LOGE(TIMER_TAG, "Failed to create traffic control timer");
    }
}