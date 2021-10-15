#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <sstream>

#include "link_state/lsa.hpp"
#include "link_state/graph.hpp"
#include "heartbeats.hpp"

using namespace boost::archive;

/** Handles socket IO **/
class Communicator
{
public:
    Communicator(int self_id);
    void listen_for_neighbors(HeartbeatsTracker &hb_tracker, PayloadHandler &payload_handler);
    void broadcast_heartbeats();

private:
    const int self_id;
    int socket_fd;
    struct sockaddr_in node_addrs[256];
    void fill_node_addrs();
    void init_socket();
    int receive_raw(char *buffer, int length, struct sockaddr_in their_addr);
    void send_raw(int target_id, const char *buffer, int length);
    void broadcast_raw(const char *buffer, int length, int exception_id);
};

class PayloadHandler
{
public:
    PayloadHandler(Graph *graph);

private:
    void send_lsa(int target_id, LSA &lsa);
};

string serialize(const LSA &obj);
LSA deserialize(const string &data);