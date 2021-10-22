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

#include "link_state/lsa.hpp"

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

class LsaSerializer
{
public:
    int serialize(const LSA &lsa, char *buffer, size_t buffer_len);
    void deserialize(const char *buffer, size_t buffer_len, LSA *lsa);

private:
    int concat_short(short int num, char *buffer);
    int concat_long(long int num, char *buffer);
    short int extract_short(const char *buffer);
    long int extract_long(const char *buffer);
};
