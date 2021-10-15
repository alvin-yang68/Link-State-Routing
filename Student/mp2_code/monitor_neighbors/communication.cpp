#include "communication.hpp"

using namespace std;

Communicator::Communicator(int self_id) : self_id{self_id}
{
    fill_node_addrs();
    init_socket();
}

void Communicator::fill_node_addrs()
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

void Communicator::init_socket()
{
    // socket() and bind() our socket. We will do all sendto()ing and recvfrom()ing on this one.
    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Communicator: socket");
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
        perror("Communicator: bind");
        close(socket_fd);
        exit(1);
    }
}

void Communicator::listen_for_neighbors(HeartbeatsTracker &hb_tracker, PayloadHandler &payload_handler)
{
    struct sockaddr_in neighbor_sockaddr;
    char neighbor_addr[100];
    char recv_buffer[5000];

    while (1)
    {
        receive_raw(recv_buffer, 5000, neighbor_sockaddr);

        inet_ntop(AF_INET, &neighbor_sockaddr.sin_addr, neighbor_addr, 100);

        short int neighbor_id = -1;
        if (strstr(neighbor_addr, "10.1.1."))
        {
            neighbor_id = atoi(
                strchr(strchr(strchr(neighbor_addr, '.') + 1, '.') + 1, '.') + 1);

            // record that we heard from neighbor_id just now.
            hb_tracker.register_heartbeat(neighbor_id);
        }

        if (!strncmp(recv_buffer, "ALIVE", 5))
            continue;

        else if (!strncmp(recv_buffer, "lsa", 3))
        {
            string payload(recv_buffer + 3);

            LSA neighbor_lsa = deserialize(payload);
            graph.accept_lsa(neighbor_lsa);

            broadcast_raw(recv_buffer, strlen(recv_buffer), neighbor_id);
        }

        // Is it a packet from the manager? (see mp2 specification for more details)
        // send format: 'send'<4 ASCII bytes>, destID<net order 2 byte signed>, <some ASCII message>
        else if (!strncmp(recv_buffer, "send", 4))
        {
            // TODO send the requested message to the requested destination node
            //  ...
        }
        //'cost'<4 ASCII bytes>, destID<net order 2 byte signed> newCost<net order 4 byte signed>
        else if (!strncmp(recv_buffer, "cost", 4))
        {
            // TODO record the cost change (remember, the link might currently be down! in that case,
            // this is the new cost you should treat it as having once it comes back up.)
            //  ...
        }

        // TODO now check for the various types of packets you use in your own protocol
        // else if(!strncmp(recv_buffer, "your other message types", ))
        //  ...
    }
    //(should never reach here)
    close(socket_fd);
}

int Communicator::receive_raw(char *buffer, int len, struct sockaddr_in neighbor_sockaddr)
{
    int bytes_received;
    socklen_t neighbor_sockaddr_len;

    if ((bytes_received = recvfrom(socket_fd, buffer, len, 0,
                                   (struct sockaddr *)&neighbor_sockaddr, &neighbor_sockaddr_len)) == -1)
    {
        perror("Communicator: recvfrom");
        exit(1);
    }

    return bytes_received;
}

void Communicator::broadcast_heartbeats()
{
    broadcast_raw("ALIVE", 5, -1);
}

void Communicator::broadcast_raw(const char *buffer, int length, int exception_id)
{
    for (int i = 0; i < 256; i++)
        if (i != self_id && i != exception_id) //(although with a real broadcast you would also get the packet yourself)
            send_raw(i, buffer, length);
}

void Communicator::send_lsa(int target_id, LSA &lsa)
{
    size_t buffer_len;
    char buffer[5000];

    buffer_len = sprintf(buffer, "lsa%s", serialize(lsa).c_str());

    send_raw(target_id, buffer, buffer_len + 1); // We add 1 to include the null terminator
}

void Communicator::send_raw(int target_id, const char *buffer, int len)
{
    if (sendto(socket_fd, buffer, len, 0,
               (struct sockaddr *)&node_addrs[target_id], sizeof(node_addrs[target_id])) == -1)
    {
        perror("Communicator: sendto");
        close(socket_fd);
        exit(1);
    }
}

string serialize(const LSA &obj)
{
    stringstream ss;
    binary_oarchive oa(ss);

    oa << obj;

    return ss.str();
}

LSA deserialize(const string &data)
{
    stringstream ss(data);
    binary_iarchive ia(ss);

    LSA obj;
    ia >> obj;

    return obj;
}