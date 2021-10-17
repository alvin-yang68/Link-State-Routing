#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <sstream>

#include "link_state/context.hpp"
#include "link_state/lsa.hpp"
#include "link_state/graph.hpp"
#include "link_state/heartbeats.hpp"
#include "log.hpp"

using namespace boost::archive;

/* Handles socket IO to other nodes */
class Socket
{
public:
    Socket(int self_id);
    size_t receive(char *buffer, int length, struct sockaddr_in their_addr);
    void send(int target_id, const char *buffer, int length);
    void broadcast(const char *buffer, int length);
    void close_socket();

private:
    const int self_id;
    int socket_fd;
    struct sockaddr_in node_addrs[256];
    void fill_node_addrs();
    void init_socket();
};

/* Handles business logic related to communication with other nodes or the manager */
class Communicator
{
public:
    Communicator(LinkStateContext *context, Log *output_log);
    void monitor_neighborhood(struct timespec heartbeat_interval, struct timespec checkup_interval, int timeout_tolerance);

private:
    Socket socket;
    LsaSerializer lsa_serializer;
    Log *output_log;
    LinkStateContext *context;
    void broadcast_heartbeats(struct timespec heartbeat_interval);
    void monitor_heartbeats(struct timespec checkup_interval, int timeout_tolerance);
    void listen_for_neighbors();
    int get_neighbor_id(const char *neighbor_addr);
    bool has_prefix(const char *recv_buffer, const char *prefix);
    int extract_int(const char *source, size_t size);
    void handle_lsa_command(const char *recv_buffer, int from_id);
    void handle_send_or_forward_command(const char *recv_buffer, bool is_from_manager);
    void handle_cost_command(const char *recv_buffer);
    void send_lsa(int target_id, LSA &lsa);
};

class LsaSerializer
{
public:
    const char *serialize(const LSA &obj);
    LSA deserialize(const char *data);
};
