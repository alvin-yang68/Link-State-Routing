#pragma once

#include <vector>
#include <queue>

#include "graph.hpp"

#define INFINITY 60000
#define UNREACHABLE -1

class RouteFinder
{
public:
    RouteFinder(uint16_t self_id);
    uint16_t find_next_hop(uint16_t destination_id);
    void run_dijkstra(Graph &graph);

private:
    const uint16_t self_id;
    vector<uint32_t> distances;
    vector<vector<uint16_t>> predecessors;
    priority_queue<EdgeToNeighbor, vector<EdgeToNeighbor>, CompareWeight> frontier;
    void clear_states();
};

template <class Q>
void clear_queue(Q &q)
{
    q = Q();
}