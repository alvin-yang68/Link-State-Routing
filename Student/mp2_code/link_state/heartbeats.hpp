#pragma once

#include <sys/time.h>
#include <unordered_set>

using namespace std;

class HeartbeatsTracker
{
public:
    HeartbeatsTracker();
    void register_heartbeat(int target_id);
    unordered_set<int> get_online_nodes(int timeout_tolerance);

private:
    struct timeval last_heartbeats[256];
    void set_initial_heartbeats();
};

// Returns the difference between two timevals in milliseconds
int subtract_timevals(struct timeval &a, struct timeval &b);