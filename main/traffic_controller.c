#include "traffic_controller.h"

traffic_state_t g_current_traffic_state = {
    .scene = TRAFFIC_SCENE_ALL_OFF,
    .blinking = false,
};

static uint8_t phase = 0;

void traffic_reset_phase(void) { phase = 0; }

static void publish_scene_group(uint16_t group_addr, traffic_scene_t scene, uint64_t execute_at_ms) {
    scene_command_t cmd = {
        .scene = scene,
        .execute_at_ms = execute_at_ms,
    };

    mesh_publish_to_group_scene_command(g_node.net_idx, vnd_models[0].keys[0], group_addr, &cmd);
}

static void publish_scene(traffic_scene_t scene, uint64_t execute_at_ms) {
    scene_command_t cmd = {
        .scene = scene,
        .execute_at_ms = execute_at_ms,
    };

    mesh_publish__scene_command(&cmd);
}

void traffic_apply_scene(traffic_scene_t scene) {
    switch (scene) {
        case TRAFFIC_SCENE_ONLY_RED:
            board_led_operation(LED_RED, LED_ON);
            board_led_operation(LED_YELLOW, LED_OFF);
            board_led_operation(LED_GREEN, LED_OFF);
            break;

        case TRAFFIC_SCENE_RED_AND_YELLOW:
            board_led_operation(LED_RED, LED_ON);
            board_led_operation(LED_YELLOW, LED_ON);
            board_led_operation(LED_GREEN, LED_OFF);
            break;

        case TRAFFIC_SCENE_ONLY_GREEN:
            board_led_operation(LED_RED, LED_OFF);
            board_led_operation(LED_YELLOW, LED_OFF);
            board_led_operation(LED_GREEN, LED_ON);
            break;

        case TRAFFIC_SCENE_ONLY_YELLOW:
            board_led_operation(LED_RED, LED_OFF);
            board_led_operation(LED_YELLOW, LED_ON);
            board_led_operation(LED_GREEN, LED_OFF);
            break;

        case TRAFFIC_SCENE_ALL_OFF:
            board_led_operation(LED_RED, LED_OFF);
            board_led_operation(LED_YELLOW, LED_OFF);
            board_led_operation(LED_GREEN, LED_OFF);
            break;
    }
}

void scheduler_step() {
    uint32_t delay_ms = 1000;
    if (!g_current_traffic_state.blinking) {
        switch (phase) {
            case 0:
                phase = 1;
                delay_ms = START_DELAY_MS;
                publish_scene(TRAFFIC_SCENE_ONLY_RED, esp_timer_get_time() / 1000 + delay_ms);
                break;

            case 1:
                phase = 2;
                delay_ms = RED_YELLOW_DURATION_MS;
                publish_scene_group(TRAFFIC_GROUP1_ADDR, TRAFFIC_SCENE_RED_AND_YELLOW, (esp_timer_get_time() / 1000) + delay_ms);
                break;

            case 2:
                phase = 3;
                delay_ms = (((uint32_t)rand()) % 5000) + 5000;
                publish_scene_group(TRAFFIC_GROUP1_ADDR, TRAFFIC_SCENE_ONLY_GREEN, (esp_timer_get_time() / 1000) + delay_ms);
                break;

            case 3:
                phase = 4;
                delay_ms = YELLOW_DURATION_MS;
                publish_scene_group(TRAFFIC_GROUP1_ADDR, TRAFFIC_SCENE_ONLY_YELLOW, (esp_timer_get_time() / 1000) + delay_ms);
                break;

            case 4:
                phase = 5;
                delay_ms = RED_DURATION_MS;
                publish_scene_group(TRAFFIC_GROUP1_ADDR, TRAFFIC_SCENE_ONLY_RED, (esp_timer_get_time() / 1000) + delay_ms);
                break;

            case 5:
                phase = 6;
                delay_ms = RED_YELLOW_DURATION_MS;
                publish_scene_group(TRAFFIC_GROUP2_ADDR, TRAFFIC_SCENE_RED_AND_YELLOW, (esp_timer_get_time() / 1000) + delay_ms);
                break;

            case 6:
                phase = 7;
                delay_ms = (((uint32_t)rand()) % 5000) + 5000;
                publish_scene_group(TRAFFIC_GROUP2_ADDR, TRAFFIC_SCENE_ONLY_GREEN, (esp_timer_get_time() / 1000) + delay_ms);
                break;

            case 7:
                phase = 8;
                delay_ms = YELLOW_DURATION_MS;
                publish_scene_group(TRAFFIC_GROUP2_ADDR, TRAFFIC_SCENE_ONLY_YELLOW, (esp_timer_get_time() / 1000) + delay_ms);
                break;

            case 8:
                phase = 1;
                delay_ms = RED_DURATION_MS;
                publish_scene_group(TRAFFIC_GROUP2_ADDR, TRAFFIC_SCENE_ONLY_RED, (esp_timer_get_time() / 1000) + delay_ms);
                break;

            default:
                break;
        }
    } else {
        if (phase == 0) {
            phase = 1;
            delay_ms = BLINK_INTERVAL_MS;
            publish_scene(TRAFFIC_SCENE_ALL_OFF, (esp_timer_get_time() / 1000) + delay_ms);
        } else {
            phase = 0;
            delay_ms = BLINK_INTERVAL_MS;
            publish_scene(TRAFFIC_SCENE_ONLY_YELLOW, (esp_timer_get_time() / 1000) + delay_ms);
        }
    }
    start_timer_once(traffic_control_timer, "traffic control timer", delay_ms * 1000 + 100000);
}

void start_scheduler() {
    if (esp_timer_is_active(traffic_control_timer)) {
        stop_timer(traffic_control_timer);
    }
    scheduler_step();
}