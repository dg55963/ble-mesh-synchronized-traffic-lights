#ifndef NODE_STATE_H
#define NODE_STATE_H

#define MAX_NODES 9
#define NODE_STATE_TAG "NODE_STATE"

#include <inttypes.h>
#include <string.h>

#include "access.h"
#include "esp_log.h"

typedef enum {
    NODE_IDLE,
    NODE_LEADER,
    NODE_FOLLOWER,
} node_role_t;

typedef struct {
    node_role_t role;

    uint16_t my_addr;
    uint16_t leader_addr;

    uint16_t slave_list[MAX_NODES - 1];
    uint16_t slave_count;
    uint8_t slave_check_vector;
} node_state_t;

extern node_state_t g_node;

void node_reset(void);
void node_become_leader(void);
void node_become_follower(uint16_t leader_addr);

bool node_is_idle(void);
bool node_is_leader(void);
bool node_has_leader(void);

void node_add_slave(uint16_t addr);
bool node_slave_exists(uint16_t addr);
void node_clear_slaves(void);
#endif  // NODE_STATE_H