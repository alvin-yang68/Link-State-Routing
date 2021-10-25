#pragma once

#include <sys/time.h>
#include <unordered_set>

#include "common/config.hpp"

using namespace std;

class HeartbeatsTracker
{
public:
    HeartbeatsTracker();
    void register_heartbeat(int target_id);
    unordered_set<int> get_online_nodes(long int timeout_tolerance_ms);

private:
    struct timeval last_heartbeats[256];
    void set_initial_heartbeats();
};

// Returns the difference between two timevals in milliseconds
long int subtract_timevals_ms(struct timeval &a, struct timeval &b);