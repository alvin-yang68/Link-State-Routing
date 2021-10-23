#include <assert.h>

#include "network.hpp"

using namespace std;

Socket::Socket(int self_id) : self_id{self_id}
{
    fill_node_addrs();
    init_socket();
}

void Socket::fill_node_addrs()
{
    for (int i = 0; i < 256; i++)
    {
        char tempaddr[100];
        sprintf(tempaddr, "10.1.1.%d", i);
        memset(&node_addrs[i], 0, sizeof(node_addrs[i]));
        node_addrs[i].sin_family = AF_INET;
        node_addrs[i].sin_port = htons(7777);
        inet_pton(AF_INET, tempaddr, &node_addrs[i].sin_addr);
    }
}

void Socket::init_socket()
{
    // socket() and bind() our socket. We will do all sendto()ing and recvfrom()ing on this one.
    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Socket: socket");
        exit(1);
    }

    char myAddr[100];
    struct sockaddr_in bindAddr;

    sprintf(myAddr, "10.1.1.%d", self_id);
    memset(&bindAddr, 0, sizeof(bindAddr));
    bindAddr.sin_family = AF_INET;
    bindAddr.sin_port = htons(7777);
    inet_pton(AF_INET, myAddr, &bindAddr.sin_addr);

    if (bind(socket_fd, (struct sockaddr *)&bindAddr, sizeof(struct sockaddr_in)) < 0)
    {
        perror("Socket: bind");
        close(socket_fd);
        exit(1);
    }
}

size_t Socket::receive(char *buffer, struct sockaddr_in *neighbor_sockaddr)
{
    size_t bytes_received;
    socklen_t neighbor_sockaddr_len = sizeof(*neighbor_sockaddr);

    if ((bytes_received = recvfrom(socket_fd, buffer, sizeof(buffer), 0,
                                   (struct sockaddr *)neighbor_sockaddr, &neighbor_sockaddr_len)) == -1)
    {
        perror("Socket: recvfrom");
        exit(1);
    }

    buffer[bytes_received] = '\0'; // Null terminate the buffer

    return bytes_received;
}

void Socket::broadcast(const char *buffer, int length)
{
    for (int i = 0; i < 256; i++)
        if (i != self_id) //(although with a real broadcast you would also get the packet yourself)
            send(i, buffer, length);
}

void Socket::send(int target_id, const char *buffer, int len)
{
    sendto(socket_fd, buffer, len, 0, (struct sockaddr *)&node_addrs[target_id], sizeof(node_addrs[target_id]));
}

void Socket::close_socket()
{
    close(socket_fd);
}

size_t LsaSerializer::serialize(const LSA &lsa, char *buffer, size_t buffer_len)
{
    size_t idx = 0;

    idx += add_short(lsa.origin_id, buffer + idx);
    idx += add_long(lsa.sequence_num, buffer + idx);

    for (const EdgeToNeighbor &edge : lsa.edges)
    {
        idx += add_short(edge.neighbor_id, buffer + idx);
        idx += add_long(edge.weight, buffer + idx);
    }

    buffer[idx] = '\0';
    return idx;
}

void LsaSerializer::deserialize(const char *buffer, size_t buffer_len, LSA *lsa)
{
    int idx = 0;

    lsa->origin_id = extract_short(buffer + idx);
    idx += sizeof(short int);

    lsa->sequence_num = extract_long(buffer + idx);
    idx += sizeof(long int);

    while (idx <= buffer_len)
    {
        int neighbor_id = extract_short(buffer + idx);
        idx += sizeof(short int);

        int edge_weight = extract_long(buffer + idx);
        idx += sizeof(long int);

        lsa->add_weight(neighbor_id, edge_weight);
    }

    assert(idx == buffer_len);
}
