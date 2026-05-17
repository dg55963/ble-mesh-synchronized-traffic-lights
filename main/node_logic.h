#ifndef NODE_LOGIC_H
#define NODE_LOGIC_H

#define NODE_LOGIC_TAG "NODE_LOGIC"

#include "esp_log.h"
#include "my_timers.h"
#include "node_state.h"

void start_heartbeat();
void start_leader_election();
void start_sync();

#endif  // NODE_LOGIC_H