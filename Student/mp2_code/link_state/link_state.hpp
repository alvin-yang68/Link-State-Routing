#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "io/network.hpp"
#include "io/log.hpp"
#include "io/utils.hpp"
#include "lsa.hpp"
#include "graph.hpp"
#include "heartbeats.hpp"

#define LSA_FREQUENCY 2
#define DIJKSTRA_FREQUENCY 2

class LinkState
{
public:
    LinkState(int self_id, const char *cost_file_name, const char *log_file_name);
    void monitor_neighborhood(struct timespec heartbeat_interval, struct timespec checkup_interval, int timeout_tolerance);

private:
    int self_id;
    Socket socket;
    LsaSerializer lsa_serializer;
    Log output_log;
    HeartbeatsTracker hb_tracker;
    Graph graph;
    RouteFinder route_finder;
    void set_initial_costs(const char *filename);
    void broadcast_heartbeats(struct timespec heartbeat_interval);
    void monitor_heartbeats(struct timespec checkup_interval, int timeout_tolerance);
    void send_lsa_to_neighbors();
    void listen_for_neighbors();
    int get_neighbor_id(const char *neighbor_addr);
    bool has_prefix(const char *recv_buffer, const char *prefix);
    void handle_lsa_command(const char *recv_buffer, size_t recv_buffer_len, int from_id);
    void handle_send_or_forward_command(const char *recv_buffer, bool is_from_manager);
    void handle_cost_command(const char *recv_buffer);
};