#include "graph.hpp"

#define INFINITY 0x3f3f3f3f
#define UNREACHABLE -1

using namespace std;

Node::Node(int id) : id{id}, sequence_num{0}, neighbors{}, edge_weights{}
{
    init_node(NUM_OF_NODES);
}

int Node::get_id()
{
    return id;
}

void Node::init_node(int max_size)
{
    neighbors.reserve(max_size);
    for (int i = 0; i < max_size; i++)
    {
        if (i == id)
            continue;
        neighbors.insert(i);
    }

    edge_weights.reserve(max_size);
    edge_weights.insert(make_pair(id, 0)); // Edge weight to itself is zero
}

int Node::get_edge_weight(int target)
{
    if (has_edge_weight(target))
        return edge_weights.at(target);

    return 1;
}

bool Node::has_edge_weight(int target)
{
    return edge_weights.find(target) != edge_weights.end();
}

void Node::set_edge_weight(int target, int new_weight)
{
    edge_weights[target] = new_weight;
}

LSA Node::generate_lsa()
{
    LSA lsa(id, sequence_num);

    for (const int &id : neighbors)
    {
        lsa.add_weight(id, get_edge_weight(id));
    }

    return lsa;
}

void Node::from_lsa(LSA &lsa)
{
    sequence_num = lsa.sequence_num;
    neighbors.clear();

    for (const struct EdgeToNeighbor &new_edge : lsa.edges)
    {
        neighbors.insert(new_edge.neighbor_id);
        set_edge_weight(new_edge.neighbor_id, new_edge.weight);
    }
}

Graph::Graph() : nodes{}, graph_mutex{}
{
    nodes.reserve(NUM_OF_NODES);
}

Graph::Graph(const Graph &other) : nodes(other.nodes), graph_mutex{}
{
}

Graph::Graph(Graph &&other) : nodes(move(other.nodes)), graph_mutex{}
{
}

Graph &Graph::operator=(Graph &other)
{
    nodes = move(other.get_nodes_map_copy()); // We move assigns the `nodes` copy returned by `get_nodes_map_copy` to avoid copying it again for the second time
    return *this;
}

unordered_map<int, Node> Graph::get_nodes_map_copy()
{
    graph_mutex.lock();
    unordered_map<int, Node> nodes_copy(nodes); // This creates a copy of `nodes` (i.e. "copy initialization")
    graph_mutex.unlock();

    return nodes_copy; // Hopefully copy elision is triggered. Otherwise, it should perform a move semantic
}

Node Graph::get_node(int id)
{
    graph_mutex.lock();
    Node node_copy(get_node_(id)); // This will perform a copy (i.e. "copy initialization") because `get_node` returns a reference that is owned by `nodes`
    graph_mutex.unlock();

    return node_copy;
}

Node &Graph::get_node_(int id)
{
    if (!has_node(id))
        nodes.emplace(id, Node(id));

    return nodes.at(id);
}

bool Graph::has_node(int target) const
{
    return nodes.find(target) != nodes.end();
}

// This steals the data from `target` node and gives their ownerships to the graph
void Graph::set_node(Node &target)
{
    graph_mutex.lock();
    nodes[target.get_id()] = move(target);
    graph_mutex.unlock();
}

bool Graph::accept_lsa(LSA &lsa)
{
    graph_mutex.lock();

    int target_id = lsa.origin_id;
    Node &target_node = get_node_(target_id); // This is NOT a copy

    bool node_updated = false;

    if (target_node.sequence_num < lsa.sequence_num)
    {
        target_node.from_lsa(lsa);
        node_updated = true;
    }

    graph_mutex.unlock();
    return node_updated;
}

RouteFinder::RouteFinder(int self_id) : self_id{self_id}
{
}

void RouteFinder::register_graph(Graph *graph)
{
    this->graph = graph;
}

void RouteFinder::run_dijkstra()
{
    ready_states();

    frontier.push({.neighbor_id = self_id, .weight = distances[self_id]});

    while (!frontier.empty())
    {
        const int curr_id = frontier.top().neighbor_id;
        frontier.pop();

        Node curr_node = graph_snapshot.get_node(curr_id);

        for (const int neighbor_id : curr_node.neighbors)
        {
            const bool is_unvisited = distances[neighbor_id] == INFINITY ? true : false;

            const int potential_weight = distances[curr_id] + curr_node.get_edge_weight(neighbor_id);

            if (potential_weight < distances[neighbor_id])
            {
                distances[neighbor_id] = potential_weight;
                predecessors[neighbor_id] = vector<int>{curr_id};
            }
            else if (potential_weight == distances[neighbor_id])
            {
                predecessors[neighbor_id].push_back(curr_id);
            }

            if (is_unvisited)
                frontier.push({.neighbor_id = neighbor_id, .weight = distances[neighbor_id]});
        }
    }
}

void RouteFinder::ready_states()
{
    distances = vector<int>(NUM_OF_NODES, INFINITY);
    distances[self_id] = 0;

    predecessors = vector<vector<int>>(NUM_OF_NODES, vector<int>());
    predecessors[self_id].push_back(self_id);

    clear_queue(frontier);

    graph_snapshot = *graph; // Copy assigns the current graph. Internally, it performs a locked copy assignment from `graph.nodes` to `graph_snapshot.nodes`
}

int RouteFinder::find_next_hop(int destination_id)
{
    if (predecessors[destination_id].empty())
        return UNREACHABLE;

    int min_next_hop = INFINITY;
    int potential_next_hop;

    for (const int pred_id : predecessors[destination_id])
    {
        if (pred_id == self_id)
            return destination_id;

        potential_next_hop = find_next_hop(pred_id);
        min_next_hop = potential_next_hop < min_next_hop ? potential_next_hop : min_next_hop;
    }

    return min_next_hop;
}
