#include "route_finder.hpp"

RouteFinder::RouteFinder(uint16_t self_id) : self_id{self_id}
{
}

void RouteFinder::run_dijkstra(Graph &graph)
{
    clear_states();

    frontier.push({.neighbor_id = self_id, .weight = distances[self_id]});

    while (!frontier.empty())
    {
        const uint16_t curr_id = frontier.top().neighbor_id;
        frontier.pop();

        Node *curr_node = graph.get_node(curr_id);

        for (const uint16_t neighbor_id : curr_node->neighbors)
        {
            const bool is_unvisited = distances[neighbor_id] == INFINITY ? true : false;

            const uint32_t potential_weight = distances[curr_id] + curr_node->get_edge_weight(neighbor_id);

            if (potential_weight < distances[neighbor_id])
            {
                distances[neighbor_id] = potential_weight;
                predecessors[neighbor_id] = vector<uint16_t>{curr_id};
            }
            else if (potential_weight == distances[neighbor_id])
            {
                predecessors[neighbor_id].push_back(curr_id);
            }

            if (is_unvisited)
                frontier.push({.neighbor_id = neighbor_id, .weight = distances[neighbor_id]});
        }
    }
}

void RouteFinder::clear_states()
{
    distances = vector<uint32_t>(NUM_OF_NODES, INFINITY);
    distances[self_id] = 0;

    predecessors = vector<vector<uint16_t>>(NUM_OF_NODES, vector<uint16_t>());
    predecessors[self_id].push_back(self_id);

    clear_queue(frontier);
}

uint16_t RouteFinder::find_next_hop(uint16_t destination_id)
{
    if (predecessors[destination_id].empty())
        return UNREACHABLE;

    uint16_t min_next_hop = INFINITY;
    uint16_t potential_next_hop;

    for (const uint16_t pred_id : predecessors[destination_id])
    {
        if (pred_id == self_id)
            return destination_id;

        potential_next_hop = find_next_hop(pred_id);
        min_next_hop = potential_next_hop < min_next_hop ? potential_next_hop : min_next_hop;
    }

    return min_next_hop;
}
