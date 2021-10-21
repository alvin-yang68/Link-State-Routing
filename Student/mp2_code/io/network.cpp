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
    if (sendto(socket_fd, buffer, len, 0,
               (struct sockaddr *)&node_addrs[target_id], sizeof(node_addrs[target_id])) == -1)
    {
        perror("Socket: sendto");
        close(socket_fd);
        exit(1);
    }
}

void Socket::close_socket()
{
    close(socket_fd);
}

void LsaSerializer::serialize(const LSA &input, char *output, size_t output_len)
{
    stringstream ss;
    binary_oarchive oa(ss);

    oa << input;

    string data = ss.str();
    memcpy(output, data.c_str(), output_len);
}

void LsaSerializer::deserialize(const char *input, LSA *output)
{
    stringstream ss(input);
    binary_iarchive ia(ss);

    ia >> *output;
}