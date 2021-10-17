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

Communicator::Communicator(LinkStateContext *context, Log *output_log) : socket{Socket(context->get_self_id())}, lsa_serializer{LsaSerializer()}, output_log{output_log}, context{context}
{
}

void Communicator::monitor_neighborhood(struct timespec heartbeat_interval, struct timespec checkup_interval, int timeout_tolerance)
{
    thread broadcast_heartbeats_thread(&Communicator::broadcast_heartbeats, this, heartbeat_interval);
    thread update_neighbors_thread(&Communicator::monitor_heartbeats, this, checkup_interval, timeout_tolerance);

    listen_for_neighbors();

    broadcast_heartbeats_thread.join();
    update_neighbors_thread.join();
}

void Communicator::broadcast_heartbeats(struct timespec heartbeat_interval)
{
    while (1)
    {
        socket.broadcast("ALIVE", 5);
        nanosleep(&heartbeat_interval, 0);
    }
}

void Communicator::monitor_heartbeats(struct timespec checkup_interval, int timeout_tolerance)
{
    HeartbeatsTracker *hb_tracker = context->get_hb_tracker();
    Node *self_node = context->get_self_node();

    while (1)
    {
        nanosleep(&checkup_interval, 0); // We sleep first because `globalLastHeartbeat` was initialized to the time when the program was started

        unordered_set<int> online_neighbors = hb_tracker->get_online_nodes(timeout_tolerance);
        if (online_neighbors != self_node->neighbors)
        {
            self_node->neighbors = online_neighbors;
            self_node->sequence_num += 1;

            LSA lsa = self_node->to_lsa();

            for (const int &id : online_neighbors)
            {
                send_lsa(id, lsa);
            }
        }
    }
}

void Communicator::listen_for_neighbors()
{
    struct sockaddr_in neighbor_sockaddr;
    char neighbor_addr[100];
    char recv_buffer[5000];

    HeartbeatsTracker *hb_tracker = context->get_hb_tracker();

    while (1)
    {
        socket.receive(recv_buffer, 5000, neighbor_sockaddr); // This will block

        // Extract the IP address of the sender and assign it to `neighbor_addr`
        inet_ntop(AF_INET, &neighbor_sockaddr.sin_addr, neighbor_addr, 100);

        short int neighbor_id = get_neighbor_id(neighbor_addr); // Set to -1 if sender was the manager
        bool is_from_manager = neighbor_id == -1 ? true : false;

        if (has_prefix(recv_buffer, "ALIVE"))
            hb_tracker->register_heartbeat(neighbor_id);

        else if (has_prefix(recv_buffer, "lsa"))
            handle_lsa_command(recv_buffer, neighbor_id);

        // We handle both the send and forward commands similarly
        else if (has_prefix(recv_buffer, "send") || has_prefix(recv_buffer, "frwd"))
            handle_send_or_forward_command(recv_buffer, is_from_manager);

        else if (has_prefix(recv_buffer, "cost"))
            handle_cost_command(recv_buffer);
    }
    //(should never reach here)
    socket.close_socket();
}

int Communicator::get_neighbor_id(const char *neighbor_addr)
{
    short int neighbor_id = -1;
    if (strstr(neighbor_addr, "10.1.1."))
    {
        neighbor_id = atoi(
            strchr(strchr(strchr(neighbor_addr, '.') + 1, '.') + 1, '.') + 1);
    }
    return neighbor_id;
}

bool Communicator::has_prefix(const char *recv_buffer, const char *prefix)
{
    return !strncmp(recv_buffer, prefix, strlen(prefix));
}

void Communicator::handle_lsa_command(const char *recv_buffer, int from_id)
{
    LSA lsa = lsa_serializer.deserialize(recv_buffer + 3);
    Graph *graph = context->get_graph();

    if (!graph->accept_lsa(lsa)) // If stale LSA, then do nothing
        return;

    Node *self_node = context->get_self_node();
    for (const int &id : self_node->neighbors)
    {
        // Forward LSA to our neighbors, but don't send the LSA back to the neighbor who sent it to us
        if (id != from_id)
            socket.send(id, recv_buffer, strlen(recv_buffer));
    }
}

void Communicator::handle_send_or_forward_command(const char *recv_buffer, bool is_from_manager)
{
    // recv_buffer format: 'send'<4 ASCII bytes>, destID<net order 2 byte signed>, <some ASCII message>
    int destination_id = extract_int(recv_buffer + 4, 2);
    const char *message = recv_buffer + 6;

    int self_id = context->get_self_id();
    if (destination_id == self_id)
    {
        output_log->add_receive_entry(message);
        return;
    }

    RouteFinder *route_finder = context->get_route_finder();
    int next_hop_id = route_finder->find_next_hop(destination_id);

    if (next_hop_id < 0)
    {
        output_log->add_unreachable_entry(destination_id);
        return;
    }

    char forward_command[5000];
    int length = sprintf(forward_command, "frwd%d%s", htons(destination_id), message);
    socket.send(next_hop_id, forward_command, length);

    if (is_from_manager)
        output_log->add_send_entry(destination_id, next_hop_id, message);
    else
        output_log->add_forward_entry(destination_id, next_hop_id, message);
}

// Extracts a number from network buffer and returns it in host order
int Communicator::extract_int(const char *network_buffer, size_t size)
{
    char number[size + 1];
    strncpy(number, network_buffer, size + 1); // `size + 1` to ensure the last char is a NULL

    if (size <= 2)
        return ntohs(atoi(number)); // short int
    else
        return ntohl(atoi(number)); // long int
}

void Communicator::handle_cost_command(const char *recv_buffer)
{
    // recv_buffer format: 'cost'<4 ASCII bytes>, destID<net order 2 byte signed> newCost<net order 4 byte signed>
    int destination_id = extract_int(recv_buffer + 4, 2);
    int new_weight = extract_int(recv_buffer + 6, 4);

    Node *self_node = context->get_self_node();
    self_node->set_edge_weight(destination_id, new_weight);
}

void Communicator::send_lsa(int target_id, LSA &lsa)
{
    size_t buffer_len;
    char buffer[5000];

    const char *serialized_lsa = lsa_serializer.serialize(lsa);
    buffer_len = sprintf(buffer, "lsa%s", serialized_lsa);

    socket.send(target_id, buffer, buffer_len);
}

const char *LsaSerializer::serialize(const LSA &obj)
{
    stringstream ss;
    binary_oarchive oa(ss);

    oa << obj;

    string data = ss.str();
    return data.c_str();
}

LSA LsaSerializer::deserialize(const char *data)
{
    stringstream ss(data);
    binary_iarchive ia(ss);

    LSA obj;
    ia >> obj;

    return obj;
}