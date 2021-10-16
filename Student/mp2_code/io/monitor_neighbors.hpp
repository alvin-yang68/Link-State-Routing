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
#include <pthread.h>

#include "topology/graph.hpp"

extern int globalMyID;
// last time you heard from each node. TODO: you will want to monitor this
// in order to realize when a neighbor has gotten cut off from you.
extern struct timeval globalLastHeartbeat[256];

// our all-purpose UDP socket, to be bound to 10.1.1.globalMyID, port 7777
extern int globalSocketUDP;
// pre-filled for sending to 10.1.1.0 - 255, port 7777
extern struct sockaddr_in globalNodeAddrs[256];

void hackyBroadcast(const char *buf, int length);
void *announceToNeighbors(void *unusedParam);
void listenForNeighbors();

void receive_lsa(char *data);
void *monitor_neighborhood_changes(void *arg);
vector<int> get_expired_nodes();
void send_to_neighbor(int destination_id, LSA &lsa);
string serialize(const LSA &obj);
LSA deserialize(const string &data);