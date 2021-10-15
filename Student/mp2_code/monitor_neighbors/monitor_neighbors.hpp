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
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>

#include "link_state/graph.hpp"
#include "utils.hpp"

#define HEARTBEAT_INTERVAL_SEC 0
#define HEARTBEAT_INTERVAL_NSEC 500 * 1000 * 1000 // 500ms
#define CHECKUP_INTERVAL_SEC 1
#define CHECKUP_INTERVAL_NSEC 500 * 1000 * 1000 // 500ms
#define TIMEOUT_TOLERANCE_MS 1000

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

void *monitor_neighborhood_changes(void *arg);
vector<int> get_expired_nodes();
void send_lsa(int destination_id, LSA &lsa);
void serialize_lsa(LSA &lsa, char *buffer);
LSA deserialize_lsa(char *buffer, int length);