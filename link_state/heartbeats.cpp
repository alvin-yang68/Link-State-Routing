#include "heartbeats.hpp"

using namespace std;

HeartbeatsTracker::HeartbeatsTracker()
{
    set_initial_heartbeats();
}

void HeartbeatsTracker::set_initial_heartbeats()
{
    for (uint16_t i = 0; i < NUM_OF_NODES; i++)
    {
        gettimeofday(&last_heartbeats[i], 0);
    }
}

void HeartbeatsTracker::register_heartbeat(uint16_t target_id)
{
    gettimeofday(&last_heartbeats[target_id], 0);
}

unordered_set<uint16_t> HeartbeatsTracker::get_online_nodes(long timeout_tolerance_ms)
{
    unordered_set<uint16_t> online_nodes;
    online_nodes.reserve(NUM_OF_NODES);

    struct timeval now;
    gettimeofday(&now, 0);

    for (uint16_t i = 0; i < NUM_OF_NODES; i++)
    {
        if (subtract_timevals_ms(now, last_heartbeats[i]) <= timeout_tolerance_ms)
            online_nodes.insert(i);
    }

    return online_nodes;
}

long subtract_timevals_ms(struct timeval &a, struct timeval &b)
{
    long diff_sec = a.tv_sec - b.tv_sec;
    long diff_us = a.tv_usec - b.tv_usec;
    return (diff_sec * 1000) + (diff_us / 1000);
}