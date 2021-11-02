#include "link_state.hpp"

using namespace std;

LinkState::LinkState(uint16_t self_id, const char *cost_file_name, const char *log_file_name) : self_node{create_self_node(self_id, cost_file_name)}, graph{Graph()}, route_finder{RouteFinder(self_id)}, graph_updated{false}, socket{Socket(self_id)}, lsa_serializer{LsaSerializer()}, output_log{Log(log_file_name)}, hb_tracker{HeartbeatsTracker()}
{
    graph.insert_node(self_node);
}

// read and parse initial costs file. default to cost 1 if no entry for a node. file may be empty.
Node *LinkState::create_self_node(uint16_t self_id, const char *filename)
{
    ifstream file(filename);
    string line;

    Node *self_node = new Node(self_id);

    while (getline(file, line))
    {
        int pair[2];

        stringstream line_stream(line);
        string substr;

        for (size_t i = 0; i < 2; i++)
        {
            getline(line_stream, substr, ' ');
            pair[i] = stoi(substr);
        }

        self_node->set_edge_weight(pair[0], pair[1]);
    }

    return self_node;
}

void LinkState::monitor_neighborhood(long heartbeat_interval_ms, long checkup_interval_ms, long timeout_tolerance_ms)
{
    thread broadcast_heartbeats_thread(&LinkState::broadcast_heartbeats, this, heartbeat_interval_ms);
    thread update_neighbors_thread(&LinkState::monitor_heartbeats, this, checkup_interval_ms, timeout_tolerance_ms);

    listen_for_neighbors();

    broadcast_heartbeats_thread.join();
    update_neighbors_thread.join();
}

void LinkState::broadcast_heartbeats(long heartbeat_interval_ms)
{
    struct timespec heartbeat_interval = generate_timespec_from_ms(heartbeat_interval_ms);

    while (1)
    {
        socket.broadcast("ALIVE", 5);
        nanosleep(&heartbeat_interval, 0);
    }
}

void LinkState::monitor_heartbeats(long checkup_interval_ms, long timeout_tolerance_ms)
{
    struct timespec checkup_interval = generate_timespec_from_ms(checkup_interval_ms);

    while (1)
    {
        nanosleep(&checkup_interval, 0); // We sleep first because `globalLastHeartbeat` was initialized to the time when the program was started

        unordered_set<uint16_t> online_neighbors = hb_tracker.get_online_nodes(timeout_tolerance_ms);
        if (online_neighbors != self_node->neighbors)
        {
            graph_mutex.lock();

            self_node->neighbors = move(online_neighbors);
            self_node->sequence_num += 1;

            send_lsa_to_neighbors();
            graph_updated = true;

            graph_mutex.unlock();
        }

        if (graph_updated)
        {
            graph_updated = false;
            update_shortest_routes();
        }
    }
}

void LinkState::send_lsa_to_neighbors()
{
    LSA lsa = self_node->generate_lsa();

    size_t serialized_lsa_len;
    char serialized_lsa[4997];
    serialized_lsa_len = lsa_serializer.serialize(lsa, serialized_lsa, sizeof(serialized_lsa));

    char buffer[5000];
    short header_len = sprintf(buffer, "lsa");
    memcpy(buffer + header_len, &serialized_lsa, serialized_lsa_len);

    for (const int id : self_node->neighbors)
    {
        socket.send(id, buffer, header_len + serialized_lsa_len);
    }
}

void LinkState::update_shortest_routes()
{
    graph_mutex.lock();
    Graph graph_copy(graph); // Performs a deep copy of the graph
    graph_mutex.unlock();

    route_finder_mutex.lock();
    route_finder.run_dijkstra(graph_copy);
    route_finder_mutex.unlock();
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

        int neighbor_id = get_neighbor_id(neighbor_addr);
        bool is_from_manager = neighbor_id == -1 ? true : false;

        if (has_prefix(recv_buffer, "ALIVE") && !is_from_manager)
            hb_tracker.register_heartbeat(neighbor_id);

        else if (has_prefix(recv_buffer, "lsa") && !is_from_manager)
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

void LinkState::handle_lsa_command(const char *recv_buffer, size_t message_len, uint16_t from_id)
{
    LSA lsa;
    lsa_serializer.deserialize(recv_buffer + 3, message_len - 3, &lsa);

    if (!graph.accept_lsa(lsa)) // If stale LSA, then do nothing and return early
        return;

    graph_updated = true;

    graph_mutex.lock();
    for (const int id : self_node->neighbors)
    {
        // Forward LSA to our neighbors, but don't send the LSA back to the neighbor who sent it to us
        if (id != from_id && id != lsa.origin_id)
            socket.send(id, recv_buffer, message_len);
    }
    graph_mutex.unlock();
}

void LinkState::handle_send_or_forward_command(const char *recv_buffer, bool is_from_manager)
{
    // recv_buffer format: 'send'<4 ASCII bytes>, destID<net order 2 byte signed>, <some ASCII message>
    uint16_t destination_id = decode_short_from_str(recv_buffer + 4);
    const char *message = recv_buffer + 6;

    if (destination_id == self_node->get_id())
    {
        output_log.add_receive_entry(message);
        return;
    }

    route_finder_mutex.lock();
    int next_hop_id = route_finder.find_next_hop(destination_id);
    route_finder_mutex.unlock();

    if (next_hop_id == UNREACHABLE)
    {
        output_log.add_unreachable_entry(destination_id);
        return;
    }

    char forward_command[5000];
    int length = 0;

    length += sprintf(forward_command, "frwd");
    length += encode_short_into_str(forward_command + length, destination_id);
    length += sprintf(forward_command + length, message);

    socket.send(next_hop_id, forward_command, length);

    if (is_from_manager)
        output_log.add_send_entry(destination_id, next_hop_id, message);
    else
        output_log.add_forward_entry(destination_id, next_hop_id, message);
}

void LinkState::handle_cost_command(const char *recv_buffer)
{
    // recv_buffer format: 'cost'<4 ASCII bytes>, destID<net order 2 byte signed> newCost<net order 4 byte signed>
    uint16_t destination_id = decode_short_from_str(recv_buffer + 4);
    uint32_t new_weight = decode_long_from_str(recv_buffer + 6);

    graph_mutex.lock();

    self_node->set_edge_weight(destination_id, new_weight);
    self_node->sequence_num += 1;

    Node *destination_node = graph.get_node(destination_id);
    destination_node->set_edge_weight(self_node->get_id(), new_weight);

    graph_updated = true;
    send_lsa_to_neighbors();

    graph_mutex.unlock();
}
