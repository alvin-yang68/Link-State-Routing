#include "monitor_neighbors.hpp"

using namespace boost::iostreams;
using namespace boost::archive;

// Yes, this is terrible. It's also terrible that, in Linux, a socket
// can't receive broadcast packets unless it's bound to INADDR_ANY,
// which we can't do in this assignment.
void hackyBroadcast(const char *buf, int length)
{
    int i;
    for (i = 0; i < 256; i++)
        if (i != globalMyID) //(although with a real broadcast you would also get the packet yourself)
            sendto(globalSocketUDP, buf, length, 0,
                   (struct sockaddr *)&globalNodeAddrs[i], sizeof(globalNodeAddrs[i]));
}

void *announceToNeighbors(void *unusedParam)
{
    struct timespec sleepFor;
    sleepFor.tv_sec = HEARTBEAT_INTERVAL_SEC;
    sleepFor.tv_nsec = HEARTBEAT_INTERVAL_NSEC;
    while (1)
    {
        hackyBroadcast("HEREIAM", 7);
        nanosleep(&sleepFor, 0);
    }
}

void listenForNeighbors()
{
    char fromAddr[100];
    struct sockaddr_in theirAddr;
    socklen_t theirAddrLen;
    unsigned char recvBuf[1000];

    int bytesRecvd;
    while (1)
    {
        theirAddrLen = sizeof(theirAddr);
        if ((bytesRecvd = recvfrom(globalSocketUDP, recvBuf, 1000, 0,
                                   (struct sockaddr *)&theirAddr, &theirAddrLen)) == -1)
        {
            perror("connectivity listener: recvfrom failed");
            exit(1);
        }

        inet_ntop(AF_INET, &theirAddr.sin_addr, fromAddr, 100);

        short int heardFrom = -1;
        if (strstr(fromAddr, "10.1.1."))
        {
            heardFrom = atoi(
                strchr(strchr(strchr(fromAddr, '.') + 1, '.') + 1, '.') + 1);

            // TODO: this node can consider heardFrom to be directly connected to it; do any such logic now.

            // record that we heard from heardFrom just now.
            gettimeofday(&globalLastHeartbeat[heardFrom], 0);
        }

        // Is it a packet from the manager? (see mp2 specification for more details)
        // send format: 'send'<4 ASCII bytes>, destID<net order 2 byte signed>, <some ASCII message>
        if (!strncmp((char *)recvBuf, "send", 4))
        {
            // TODO send the requested message to the requested destination node
            //  ...
        }
        //'cost'<4 ASCII bytes>, destID<net order 2 byte signed> newCost<net order 4 byte signed>
        else if (!strncmp((char *)recvBuf, "cost", 4))
        {
            // TODO record the cost change (remember, the link might currently be down! in that case,
            // this is the new cost you should treat it as having once it comes back up.)
            //  ...
        }

        // TODO now check for the various types of packets you use in your own protocol
        // else if(!strncmp(recvBuf, "your other message types", ))
        //  ...
        else if (!strncmp((char *)recvBuf, "lsa_", 4))
        {
        }
    }
    //(should never reach here)
    close(globalSocketUDP);
}

void *monitor_neighborhood_changes(void *arg)
{
    Node *self_node = (Node *)arg;

    struct timespec sleep_duration;
    sleep_duration.tv_sec = CHECKUP_INTERVAL_SEC;
    sleep_duration.tv_nsec = CHECKUP_INTERVAL_NSEC;

    nanosleep(&sleep_duration, 0); // We sleep first because `globalLastHeartbeat` was initialized to the time when the program was started

    while (1)
    {
        unordered_set<int> online_neighbors = get_online_nodes();
        unordered_set<int> existing_neighbors = self_node->get_neighbors();

        if (online_neighbors != existing_neighbors)
        {
            self_node->set_neighbors(online_neighbors);
            self_node->sequence_num += 1;

            LSA lsa = self_node->generate_lsa();

            for (const int &id : online_neighbors)
            {
                send_lsa(id, lsa);
            }
        }

        nanosleep(&sleep_duration, 0);
    }
}

unordered_set<int> get_online_nodes()
{
    unordered_set<int> online_nodes;
    online_nodes.reserve(256);

    struct timeval now;
    gettimeofday(&now, 0);

    for (int i = 0; i < 256; i++)
    {
        if (subtract_timevals(now, globalLastHeartbeat[i]) <= TIMEOUT_TOLERANCE_MS)
            online_nodes.insert(i);
    }

    return online_nodes;
}

void send_lsa(int destination_id, LSA &lsa)
{
    size_t buffer_len = 5000;
    char buffer[buffer_len];

    serialize_lsa(lsa, buffer_len, buffer);

    sendto(globalSocketUDP, buffer, buffer_len, 0,
           (struct sockaddr *)&globalNodeAddrs[destination_id], sizeof(globalNodeAddrs[destination_id]));
}

void serialize_lsa(LSA &lsa, int length, char *buffer)
{
    basic_array_sink<char> sink(buffer, length);
    stream<basic_array_sink<char>> source(sink);
    binary_oarchive oa(source);

    oa << lsa;
}

LSA deserialize_lsa(char *buffer, int length)
{
    stream<basic_array_source<char>> source(buffer);
    binary_iarchive ia(source);

    LSA lsa;
    ia >> lsa;

    return lsa;
}