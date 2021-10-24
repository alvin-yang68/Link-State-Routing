#include "heartbeats.hpp"

using namespace std;

HeartbeatsTracker::HeartbeatsTracker()
{
    set_initial_heartbeats();
}

void HeartbeatsTracker::set_initial_heartbeats()
{
    for (int i = 0; i < 256; i++)
    {
        gettimeofday(&last_heartbeats[i], 0);
    }
}

void HeartbeatsTracker::register_heartbeat(int target_id)
{
    gettimeofday(&last_heartbeats[target_id], 0);
}

unordered_set<int> HeartbeatsTracker::get_online_nodes(long int timeout_tolerance_ms)
{
    unordered_set<int> online_nodes;
    online_nodes.reserve(256);

    struct timeval now;
    gettimeofday(&now, 0);

    for (int i = 0; i < 256; i++)
    {
        if (subtract_timevals_ms(now, last_heartbeats[i]) <= timeout_tolerance_ms)
            online_nodes.insert(i);
    }

    return online_nodes;
}

long int subtract_timevals_ms(struct timeval &a, struct timeval &b)
{
    long int diff_sec = a.tv_sec - b.tv_sec;
    long int diff_us = a.tv_usec - b.tv_usec;
    return (diff_sec * 1000) + (diff_us / 1000);
}