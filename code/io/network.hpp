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

#include "lsa.hpp"
#include "common/config.hpp"

/* Handles socket IO to other nodes */
class Socket
{
public:
    Socket(uint16_t self_id);
    size_t receive(char *buffer, size_t buffer_len, struct sockaddr_in *their_addr);
    void send(uint16_t target_id, const char *buffer, size_t length);
    void broadcast(const char *buffer, size_t length);
    void close_socket();

private:
    const uint16_t self_id;
    int socket_fd;
    struct sockaddr_in node_addrs[NUM_OF_NODES];
    void fill_node_addrs();
    void init_socket();
};
