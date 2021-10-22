#include "link_state.hpp"

LinkState::LinkState(int self_id, const char *cost_file_name, const char *log_file_name) : self_id{self_id}, socket{Socket(self_id)}, lsa_serializer{LsaSerializer()}, output_log{Log(log_file_name)}, hb_tracker{HeartbeatsTracker()}, graph{Graph()}, route_finder{RouteFinder(self_id)}
{
    graph.register_node(new Node(self_id));
    route_finder.register_graph(&graph);
    set_initial_costs(cost_file_name);
}

// read and parse initial costs file. default to cost 1 if no entry for a node. file may be empty.
void LinkState::set_initial_costs(const char *filename)
{
    ifstream file(filename);
    string line;

    Node *self_node = graph.get_node(self_id);

    while (getline(file, line))
    {
        int pair[2];

        stringstream line_stream(line);
        string substr;

        for (int i = 0; i < 2; i++)
        {
            getline(line_stream, substr, ' ');
            pair[i] = stoi(substr);
        }

        self_node->insert_neighbor(pair[0]);
        self_node->set_edge_weight(pair[0], pair[1]);
    }
}

void LinkState::monitor_neighborhood(struct timespec heartbeat_interval, struct timespec checkup_interval, int timeout_tolerance)
{
    thread listen_for_neighbors_thread(&LinkState::listen_for_neighbors, this);
    thread broadcast_heartbeats_thread(&LinkState::broadcast_heartbeats, this, heartbeat_interval);
    thread update_neighbors_thread(&LinkState::monitor_heartbeats, this, checkup_interval, timeout_tolerance);

    send_initial_lsa();

    listen_for_neighbors_thread.join();
    broadcast_heartbeats_thread.join();
    update_neighbors_thread.join();
}

void LinkState::send_initial_lsa()
{
    struct timespec sleep_interval;
    sleep_interval.tv_sec = 0;
    sleep_interval.tv_nsec = 1000;

    nanosleep(&sleep_interval, 0);
    send_lsa_to_neighbors();
}

void LinkState::send_lsa_to_neighbors()
{
    Node *self_node = graph.get_node(self_id);
    LSA lsa = self_node->generate_lsa();

    char serialized_lsa[4997];
    lsa_serializer.serialize(lsa, serialized_lsa, 4997);

    size_t buffer_len;
    char buffer[5000];
    buffer_len = sprintf(buffer, "lsa", serialized_lsa);

    for (const int &id : self_node->neighbors)
    {
        socket.send(id, buffer, buffer_len);
    }
}

void LinkState::broadcast_heartbeats(struct timespec heartbeat_interval)
{
    while (1)
    {
        socket.broadcast("ALIVE", 5);
        nanosleep(&heartbeat_interval, 0);
    }
}

void LinkState::monitor_heartbeats(struct timespec checkup_interval, int timeout_tolerance)
{
    Node *self_node = graph.get_node(self_id);

    unsigned int counter = 0;

    while (1)
    {
        nanosleep(&checkup_interval, 0); // We sleep first because `globalLastHeartbeat` was initialized to the time when the program was started

        unordered_set<int> online_neighbors = hb_tracker.get_online_nodes(timeout_tolerance);
        if (online_neighbors != self_node->neighbors)
        {
            self_node->neighbors = online_neighbors;
            self_node->sequence_num += 1;

            if (counter % LSA_FREQUENCY == 0)
                send_lsa_to_neighbors();

            if (counter % DIJKSTRA_FREQUENCY == 0)
                route_finder.run_dijkstra();
        }

        counter++;
    }
}

void LinkState::listen_for_neighbors()
{
    struct sockaddr_in neighbor_sockaddr;
    char neighbor_addr[100];
    char recv_buffer[5000];
    size_t recv_buffer_len;

    while (1)
    {
        recv_buffer_len = socket.receive(recv_buffer, 5000, neighbor_sockaddr); // This will block

        // Extract the IP address of the sender and assign it to `neighbor_addr`
        inet_ntop(AF_INET, &neighbor_sockaddr.sin_addr, neighbor_addr, 100);

        short int neighbor_id = get_neighbor_id(neighbor_addr); // Set to -1 if sender was the manager
        bool is_from_manager = neighbor_id == -1 ? true : false;

        if (has_prefix(recv_buffer, "ALIVE"))
            hb_tracker.register_heartbeat(neighbor_id);

        else if (has_prefix(recv_buffer, "lsa"))
            handle_lsa_command(recv_buffer, recv_buffer_len, neighbor_id);

        // We handle both the send and forward commands similarly
        else if (has_prefix(recv_buffer, "send") || has_prefix(recv_buffer, "frwd"))
            handle_send_or_forward_command(recv_buffer, is_from_manager);

        else if (has_prefix(recv_buffer, "cost"))
            handle_cost_command(recv_buffer);
    }
    //(should never reach here)
    socket.close_socket();
}

int LinkState::get_neighbor_id(const char *neighbor_addr)
{
    short int neighbor_id = -1;
    if (strstr(neighbor_addr, "10.1.1."))
    {
        neighbor_id = atoi(
            strchr(strchr(strchr(neighbor_addr, '.') + 1, '.') + 1, '.') + 1);
    }
    return neighbor_id;
}

bool LinkState::has_prefix(const char *recv_buffer, const char *prefix)
{
    return !strncmp(recv_buffer, prefix, strlen(prefix));
}

void LinkState::handle_lsa_command(const char *recv_buffer, size_t recv_buffer_len, int from_id)
{
    LSA lsa;
    lsa_serializer.deserialize(recv_buffer + 3, recv_buffer_len, &lsa);

    if (!graph.accept_lsa(lsa)) // If stale LSA, then do nothing
        return;

    Node *self_node = graph.get_node(self_id);
    for (const int &id : self_node->neighbors)
    {
        // Forward LSA to our neighbors, but don't send the LSA back to the neighbor who sent it to us
        if (id != from_id)
            socket.send(id, recv_buffer, strlen(recv_buffer));
    }
}

void LinkState::handle_send_or_forward_command(const char *recv_buffer, bool is_from_manager)
{
    // recv_buffer format: 'send'<4 ASCII bytes>, destID<net order 2 byte signed>, <some ASCII message>
    int destination_id = extract_int(recv_buffer + 4, 2);
    const char *message = recv_buffer + 6;

    if (destination_id == self_id)
    {
        output_log.add_receive_entry(message);
        return;
    }

    int next_hop_id = route_finder.find_next_hop(destination_id);

    if (next_hop_id < 0)
    {
        output_log.add_unreachable_entry(destination_id);
        return;
    }

    char forward_command[5000];
    int length = sprintf(forward_command, "frwd%d%s", htons(destination_id), message);
    socket.send(next_hop_id, forward_command, length);

    if (is_from_manager)
        output_log.add_send_entry(destination_id, next_hop_id, message);
    else
        output_log.add_forward_entry(destination_id, next_hop_id, message);
}

// Extracts a number from network buffer and returns it in host order
int LinkState::extract_int(const char *network_buffer, size_t size)
{
    char number[size + 1];
    strncpy(number, network_buffer, size + 1); // `size + 1` to ensure the last char is a NULL

    // TODO: Divide this method into short and long versions
    if (size <= 2)
        return ntohs(atoi(number)); // short int
    else
        return ntohl(atoi(number)); // long int
}

void LinkState::handle_cost_command(const char *recv_buffer)
{
    // recv_buffer format: 'cost'<4 ASCII bytes>, destID<net order 2 byte signed> newCost<net order 4 byte signed>
    int destination_id = extract_int(recv_buffer + 4, 2);
    int new_weight = extract_int(recv_buffer + 6, 4);

    graph.set_edge_weight_pairs(self_id, destination_id, new_weight);
}