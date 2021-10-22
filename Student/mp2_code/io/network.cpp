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

size_t Socket::receive(char *buffer, int len, struct sockaddr_in neighbor_sockaddr)
{
    size_t bytes_received;
    socklen_t neighbor_sockaddr_len;

    if ((bytes_received = recvfrom(socket_fd, buffer, len, 0,
                                   (struct sockaddr *)&neighbor_sockaddr, &neighbor_sockaddr_len)) == -1)
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

int LsaSerializer::serialize(const LSA &lsa, char *buffer, size_t buffer_len)
{
    int current_len = 0;

    current_len += concat_short(lsa.origin_id, buffer + current_len);
    current_len += concat_long(lsa.sequence_num, buffer + current_len);

    for (const EdgeToNeighbor &edge : lsa.edges)
    {
        current_len += concat_short(edge.neighbor_id, buffer + current_len);
        current_len += concat_long(edge.weight, buffer + current_len);
    }

    buffer[current_len] = '\0';
    return current_len;
}

int LsaSerializer::concat_short(short int num, char *buffer)
{
    short int network_num = htons(num);
    memcpy(buffer, &network_num, sizeof(short int));
    return sizeof(short int);
}

int LsaSerializer::concat_long(long int num, char *buffer)
{
    long int network_num = htonl(num);
    memcpy(buffer, &network_num, sizeof(long int));
    return sizeof(long int);
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

short int LsaSerializer::extract_short(const char *buffer)
{
    char num_str[sizeof(short int) + 1];
    memcpy(num_str, buffer, sizeof(short int));
    num_str[sizeof(short int)] = '\0';

    return ntohs(atoi(num_str));
}

long int LsaSerializer::extract_long(const char *buffer)
{
    char num_str[sizeof(long int) + 1];
    memcpy(num_str, buffer, sizeof(long int));
    num_str[sizeof(long int)] = '\0';

    return ntohl(atoi(num_str));
}