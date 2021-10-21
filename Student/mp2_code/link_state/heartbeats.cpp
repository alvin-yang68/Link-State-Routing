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

unordered_set<int> HeartbeatsTracker::get_online_nodes(int timeout_tolerance)
{
    unordered_set<int> online_nodes;
    online_nodes.reserve(256);

    struct timeval now;
    gettimeofday(&now, 0);

    for (int i = 0; i < 256; i++)
    {
        if (subtract_timevals(now, last_heartbeats[i]) <= (timeout_tolerance * 1000))
            online_nodes.insert(i);
    }

    return online_nodes;
}

int subtract_timevals(struct timeval &a, struct timeval &b)
{
    int diff_sec = a.tv_sec - b.tv_sec;
    int diff_us = (int)a.tv_usec - (int)b.tv_usec;
    return (diff_sec * 1000) + (diff_us / 1000);
}