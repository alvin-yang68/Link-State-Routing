#include "graph.hpp"

using namespace std;

Node::Node(uint16_t id) : id{id}, sequence_num{0}, neighbors{}, edge_weights{}
{
    neighbors.reserve(NUM_OF_NODES);
    for (uint16_t i = 0; i < NUM_OF_NODES; i++)
    {
        if (i == id)
            continue;
        neighbors.insert(i);
    }

    edge_weights.reserve(NUM_OF_NODES);
    edge_weights.insert(make_pair(id, 0)); // Edge weight to itself is zero
}

uint16_t Node::get_id() const
{
    return id;
}

uint32_t Node::get_edge_weight(uint16_t target)
{
    if (has_edge_weight(target))
        return edge_weights.at(target);

    return 1;
}

bool Node::has_edge_weight(uint16_t target)
{
    return edge_weights.find(target) != edge_weights.end();
}

void Node::set_edge_weight(uint16_t target, uint32_t new_weight)
{
    edge_weights[target] = new_weight;
}

LSA Node::generate_lsa()
{
    LSA lsa(id, sequence_num);

    for (const uint16_t id : neighbors)
    {
        lsa.add_edge(id, get_edge_weight(id));
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

Graph::Graph() : nodes{}
{
    nodes.reserve(NUM_OF_NODES);
}

Node *Graph::get_node(uint16_t id)
{
    if (!has_node(id))
        nodes.insert(make_pair(id, new Node(id)));

    return nodes.at(id);
}

void Graph::insert_node(Node *new_node)
{
    uint16_t id = new_node->get_id();
    nodes.insert(make_pair(id, new_node));
}

bool Graph::has_node(uint16_t target) const
{
    return nodes.find(target) != nodes.end();
}

bool Graph::accept_lsa(LSA &lsa)
{
    uint16_t target_id = lsa.origin_id;
    Node *target_node = get_node(target_id);

    bool node_updated = false;

    if (target_node->sequence_num < lsa.sequence_num)
    {
        target_node->from_lsa(lsa);
        node_updated = true;
    }

    return node_updated;
}
