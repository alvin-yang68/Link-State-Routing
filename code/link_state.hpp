#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <mutex>

#include "io/network.hpp"
#include "io/log.hpp"
#include "common/utils.hpp"
#include "common/config.hpp"
#include "topology/graph.hpp"
#include "topology/route_finder.hpp"
#include "lsa.hpp"
#include "heartbeats.hpp"

class LinkState
{
public:
    LinkState(uint16_t self_id, const char *cost_file_name, const char *log_file_name);
    void monitor_neighborhood(long heartbeat_interval_ms, long checkup_interval_ms, long timeout_tolerance_ms);

private:
    bool graph_updated;
    mutex graph_mutex;
    mutex route_finder_mutex;
    Node *self_node;
    Graph graph;
    RouteFinder route_finder;
    Socket socket;
    LsaSerializer lsa_serializer;
    Log output_log;
    HeartbeatsTracker hb_tracker;
    Node *create_self_node(uint16_t self_id, const char *filename);
    void broadcast_heartbeats(long heartbeat_interval_ms);
    void monitor_heartbeats(long checkup_interval_ms, long timeout_tolerance_ms);
    void send_lsa_to_neighbors();
    void update_shortest_routes();
    void listen_for_neighbors();
    int get_neighbor_id(const char *neighbor_addr);
    void handle_lsa_command(const char *recv_buffer, size_t recv_buffer_len, uint16_t from_id);
    void handle_send_or_forward_command(const char *recv_buffer, bool is_from_manager);
    void handle_cost_command(const char *recv_buffer);
};
