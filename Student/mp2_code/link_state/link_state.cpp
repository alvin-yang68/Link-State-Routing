#include "link_state.hpp"

LinkState::LinkState(int self_id, const char *cost_file_name, const char *log_file_name) : self_id{self_id}, graph_updated{false}, socket{Socket(self_id)}, lsa_serializer{LsaSerializer()}, output_log{Log(log_file_name)}, hb_tracker{HeartbeatsTracker()}, graph{Graph()}, route_finder{RouteFinder(self_id)}
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

        self_node->set_edge_weight(pair[0], pair[1]);
    }
}

void LinkState::monitor_neighborhood(long int heartbeat_interval_ms, long int checkup_interval_ms, long int timeout_tolerance_ms)
{
    thread broadcast_heartbeats_thread(&LinkState::broadcast_heartbeats, this, heartbeat_interval_ms);
    thread update_neighbors_thread(&LinkState::monitor_heartbeats, this, checkup_interval_ms, timeout_tolerance_ms);

    listen_for_neighbors();

    broadcast_heartbeats_thread.join();
    update_neighbors_thread.join();
}

void LinkState::broadcast_heartbeats(long int heartbeat_interval_ms)
{
    struct timespec heartbeat_interval = generate_timespec_from_ms(heartbeat_interval_ms);

    while (1)
    {
        socket.broadcast("ALIVE", 5);
        nanosleep(&heartbeat_interval, 0);
    }
}

void LinkState::monitor_heartbeats(long int checkup_interval_ms, long int timeout_tolerance_ms)
{
    Node *self_node = graph.get_node(self_id);

    struct timespec checkup_interval = generate_timespec_from_ms(checkup_interval_ms);

    while (1)
    {
        nanosleep(&checkup_interval, 0); // We sleep first because `globalLastHeartbeat` was initialized to the time when the program was started

        unordered_set<int> online_neighbors = hb_tracker.get_online_nodes(timeout_tolerance_ms);
        // if (!online_neighbors.empty())
        // {
        //     std::cout << counter << " online_neighbors: ";
        //     for (auto it = online_neighbors.begin(); it != online_neighbors.end(); ++it)
        //         std::cout << " " << *it;
        //     std::cout << std::endl;
        //     std::cout << counter << " self_node->neighbors: ";
        //     for (auto it = self_node->neighbors.begin(); it != self_node->neighbors.end(); ++it)
        //         std::cout << " " << *it;
        //     std::cout << std::endl;
        // }
        if (online_neighbors != self_node->neighbors)
        {
            self_node->neighbors = online_neighbors;
            self_node->sequence_num += 1;
            send_lsa_to_neighbors();

            graph_updated = true;

            // struct timeval now;
            // gettimeofday(&now, 0);
            // std::cout << now.tv_sec << ", " << now.tv_usec;
            // std::cout << ", I am: " << self_node->id;
            // std::cout << ", my neighbors: ";
            // for (const int id : online_neighbors)
            //     std::cout << id << " ";
            // std::cout << std::endl;
        }

        if (graph_updated)
        {
            graph_updated = false;
            route_finder.run_dijkstra();
        }
    }
}

void LinkState::send_lsa_to_neighbors()
{
    Node *self_node = graph.get_node(self_id);
    LSA lsa = self_node->generate_lsa();

    size_t serialized_len;
    char serialized_lsa[4997];
    serialized_len = lsa_serializer.serialize(lsa, serialized_lsa, sizeof(serialized_lsa));

    char buffer[5000];
    sprintf(buffer, "lsa");
    memcpy(buffer + 3, &serialized_lsa, serialized_len);

    for (const int &id : self_node->neighbors)
    {
        socket.send(id, buffer, 3 + serialized_len);
    }
}

void LinkState::listen_for_neighbors()
{
    struct sockaddr_in neighbor_sockaddr;
    char neighbor_addr[100];
    char recv_buffer[5000];
    size_t message_len;

    while (1)
    {
        message_len = socket.receive(recv_buffer, sizeof(recv_buffer), &neighbor_sockaddr); // This will block

        // Extract the IP address of the sender and assign it to `neighbor_addr`
        inet_ntop(AF_INET, &neighbor_sockaddr.sin_addr, neighbor_addr, 100);

        int neighbor_id = get_neighbor_id(neighbor_addr); // Set to -1 if sender was the manager
        bool is_from_manager = neighbor_id == -1 ? true : false;

        if (has_prefix(recv_buffer, "ALIVE"))
            hb_tracker.register_heartbeat(neighbor_id);

        else if (has_prefix(recv_buffer, "lsa"))
            handle_lsa_command(recv_buffer, message_len, neighbor_id);

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
    int neighbor_id = -1;
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

void LinkState::handle_lsa_command(const char *recv_buffer, size_t message_len, int from_id)
{
    LSA lsa;
    lsa_serializer.deserialize(recv_buffer + 3, message_len - 3, &lsa);

    // std::cout << "I am: " << self_id;
    // std::cout << ", received from: " << from_id;
    // std::cout << ", origin id: " << lsa.origin_id;
    // std::cout << ", seq num: " << lsa.sequence_num;
    // std::cout << ", for neighbors: ";
    // for (const struct EdgeToNeighbor edge : lsa.edges)
    //     std::cout << edge.neighbor_id << " " << edge.weight << ", ";
    // std::cout << std::endl;

    if (!graph.accept_lsa(lsa)) // If stale LSA, then do nothing
        return;

    graph_updated = true;

    // std::cout << "I am: " << self_id;
    // std::cout << ", received from: " << from_id;
    // std::cout << ", origin id: " << lsa.origin_id;
    // std::cout << ", seq num: " << lsa.sequence_num;
    // std::cout << std::endl;

    Node *self_node = graph.get_node(self_id);
    unordered_set<int> neighbors = self_node->neighbors; // Copy to avoid data race

    struct timeval now;
    gettimeofday(&now, 0);
    std::cout << now.tv_sec << ", " << now.tv_usec;
    std::cout << ", I am: " << self_id;
    std::cout << ", received from: " << from_id;
    std::cout << ", origin id: " << lsa.origin_id;
    std::cout << ", seq num: " << lsa.sequence_num;
    std::cout << ", my neighbors: ";
    for (const int id : self_node->neighbors)
        std::cout << id << " ";
    std::cout << std::endl;

    for (const int id : neighbors)
    {
        // Forward LSA to our neighbors, but don't send the LSA back to the neighbor who sent it to us
        if (id != from_id && id != lsa.origin_id)
        {
            // struct timeval now;
            // gettimeofday(&now, 0);
            // std::cout << now.tv_sec << ", " << now.tv_usec;
            // std::cout << ", I am: " << self_id;
            // std::cout << ", forwarding to: " << id;
            // std::cout << ", received from: " << from_id;
            // std::cout << ", origin id: " << lsa.origin_id;
            // std::cout << ", seq num: " << lsa.sequence_num;
            // std::cout << ", my neighbors: ";
            // for (const int id : self_node->neighbors)
            // std::cout << id << " ";
            // std::cout << ", for neighbors: ";
            // for (const struct EdgeToNeighbor edge : lsa.edges)
            //     std::cout << edge.neighbor_id << " " << edge.weight << ", ";
            // std::cout << std::endl;

            socket.send(id, recv_buffer, message_len);
        }
    }

    // route_finder.run_dijkstra();
}

void LinkState::handle_send_or_forward_command(const char *recv_buffer, bool is_from_manager)
{
    // recv_buffer format: 'send'<4 ASCII bytes>, destID<net order 2 byte signed>, <some ASCII message>
    uint16_t destination_id = extract_short(recv_buffer + 4);
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
    int length = 0;

    length += sprintf(forward_command, "frwd");
    length += add_short(destination_id, forward_command + length);
    strcpy(forward_command + length, message);
    length += strlen(message);

    socket.send(next_hop_id, forward_command, length);

    if (is_from_manager)
        output_log.add_send_entry(destination_id, next_hop_id, message);
    else
        output_log.add_forward_entry(destination_id, next_hop_id, message);
}

void LinkState::handle_cost_command(const char *recv_buffer)
{
    // recv_buffer format: 'cost'<4 ASCII bytes>, destID<net order 2 byte signed> newCost<net order 4 byte signed>
    uint16_t destination_id = extract_short(recv_buffer + 4);
    uint32_t new_weight = extract_long(recv_buffer + 6);

    graph.set_edge_weight_pairs(self_id, destination_id, new_weight);
}

struct timespec generate_timespec_from_ms(long int ms)
{
    long int floor_sec = ms / 1000;
    long int remainder_ms = ms - (floor_sec * 1000);

    struct timespec interval;
    interval.tv_sec = floor_sec;
    interval.tv_nsec = remainder_ms * 1000000;

    return interval;
}