#pragma once

#include <sys/time.h>
#include <unordered_set>

#include "common/config.hpp"

using namespace std;

class HeartbeatsTracker
{
public:
    HeartbeatsTracker();
    void register_heartbeat(uint16_t target_id);
    unordered_set<uint16_t> get_online_nodes(long timeout_tolerance_ms);

private:
    struct timeval last_heartbeats[NUM_OF_NODES];
    void set_initial_heartbeats();
};

// Returns the difference between two timevals in milliseconds
long subtract_timevals_ms(struct timeval &a, struct timeval &b);