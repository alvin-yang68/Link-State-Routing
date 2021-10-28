#include "network.hpp"

using namespace std;

Socket::Socket(uint16_t self_id) : self_id{self_id}
{
    fill_node_addrs();
    init_socket();
}

void Socket::fill_node_addrs()
{
    for (uint16_t i = 0; i < NUM_OF_NODES; i++)
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

size_t Socket::receive(char *buffer, size_t buffer_len, struct sockaddr_in *neighbor_sockaddr)
{
    size_t bytes_received;
    socklen_t neighbor_sockaddr_len = sizeof(*neighbor_sockaddr);

    if ((bytes_received = recvfrom(socket_fd, buffer, buffer_len, 0,
                                   (struct sockaddr *)neighbor_sockaddr, &neighbor_sockaddr_len)) == -1)
    {
        perror("Socket: recvfrom");
        exit(1);
    }

    buffer[bytes_received] = '\0'; // Null terminate the buffer

    return bytes_received;
}

void Socket::broadcast(const char *buffer, size_t length)
{
    for (uint16_t i = 0; i < NUM_OF_NODES; i++)
        if (i != self_id) //(although with a real broadcast you would also get the packet yourself)
            send(i, buffer, length);
}

void Socket::send(uint16_t target_id, const char *buffer, size_t len)
{
    sendto(socket_fd, buffer, len, 0, (struct sockaddr *)&node_addrs[target_id], sizeof(node_addrs[target_id]));
}

void Socket::close_socket()
{
    close(socket_fd);
}
