#include <stdexcept>

#include "graph.hpp"

#define INFINITY 0x3f3f3f3f
#define UNKNOWN -1

using namespace std;

Node::Node(int id) : id{id}, sequence_num{0}, neighbors{}, edge_weights{}
{
    edge_weights.insert(make_pair(id, 0)); // Edge weight to itself is zero
}

Node::Node(const Node &obj) : id{obj.id}, sequence_num{obj.sequence_num}, neighbors(obj.neighbors), edge_weights(obj.edge_weights)
{
}

void Node::insert_neighbor(int target)
{
    neighbors.insert(target);
}

int Node::get_edge_weight(int target)
{
    try
    {
        return edge_weights.at(target);
    }
    catch (const out_of_range &e)
    {
        return 1;
    }
}

void Node::set_edge_weight(int target, int new_weight)
{
    edge_weights[target] = new_weight;
}

void Node::set_edge_weights(vector<struct EdgeToNeighbor> new_edges)
{
    for (struct EdgeToNeighbor &edge : new_edges)
    {
        set_edge_weight(edge.neighbor_id, edge.weight);
    }
}

LSA Node::generate_lsa()
{
    LSA lsa = LSA(id, sequence_num);

    for (const int &id : neighbors)
    {
        lsa.add_weight(id, get_edge_weight(id));
    }

    return lsa;
}

Node *Graph::get_node(int id)
{
    if (has_node(id))
        return nodes.at(id);

    return register_node(new Node(id));
}

bool Graph::has_node(int target)
{
    return nodes.find(target) != nodes.end();
}

Node *Graph::register_node(Node *node)
{
    nodes.insert(make_pair(node->id, node));
    return node;
}

void Graph::set_edge_weight_pairs(int source_id, int target_id, int new_weight)
{
    Node *source_node = get_node(source_id);
    source_node->set_edge_weight(target_id, new_weight);

    Node *target_node = get_node(target_id);
    target_node->set_edge_weight(source_id, new_weight);
}

bool Graph::accept_lsa(LSA &lsa)
{
    int target_id = lsa.origin_id;

    Node *target_node = get_node(target_id);

    if (lsa.sequence_num <= target_node->sequence_num)
        return false;

    target_node->sequence_num = lsa.sequence_num;
    target_node->neighbors = lsa.get_neighbors();

    for (const struct EdgeToNeighbor &new_edge : lsa.edges)
    {
        target_node->set_edge_weight(new_edge.neighbor_id, new_edge.weight);
    }

    return true;
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
    reset_states();
    frontier.push({.neighbor_id = self_id, .weight = distances[self_id]});

    while (!frontier.empty())
    {
        const int curr_id = frontier.top().neighbor_id;
        frontier.pop();

        Node *curr_node = graph->get_node(curr_id);

        for (const int neighbor_id : curr_node->neighbors)
        {
            const bool is_unvisited = distances[neighbor_id] == INFINITY ? true : false;

            const int potential_weight = distances[curr_id] + curr_node->get_edge_weight(neighbor_id);

            if (potential_weight < distances[neighbor_id])
            {
                distances[neighbor_id] = potential_weight;
                predecessors[neighbor_id] = curr_id;
            }

            if (is_unvisited)
                frontier.push({.neighbor_id = neighbor_id, .weight = distances[neighbor_id]});
        }
    }
}

void RouteFinder::reset_states()
{
    distances = vector<int>(NUM_OF_NODES, INFINITY);
    distances[self_id] = 0;

    predecessors = vector<int>(NUM_OF_NODES, UNKNOWN);
    predecessors[self_id] = self_id;

    clear_queue(frontier);
}

int RouteFinder::find_next_hop(int destination_id)
{
    int curr_id = destination_id;

    while (predecessors[curr_id] != self_id && curr_id != -1)
    {
        curr_id = predecessors[curr_id];
    }

    return curr_id;
}