#include <stdexcept>

#include "graph.hpp"

using namespace std;

Node::Node(int id) : id{id}, sequence_num{0}, neighbors{}, edge_weights{}
{
    edge_weights.insert(make_pair(id, 0)); // Edge weight to itself is zero
}

Node::Node(const Node &obj) : id{obj.id}, sequence_num{obj.sequence_num}, neighbors(obj.neighbors), edge_weights(obj.edge_weights)
{
}

bool Node::has_neighbor(int target)
{
    return neighbors.find(target) == neighbors.end();
}

unordered_set<int> Node::get_neighbors()
{
    return neighbors;
}

void Node::set_neighbors(const unordered_set<int> &new_neighbors)
{
    neighbors = new_neighbors;
}

int Node::get_edge_weight(int target)
{
    if (!has_neighbor(target))
        throw out_of_range("non-existent edge");

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
    try
    {
        edge_weights.at(target) = new_weight;
    }
    catch (const out_of_range &e)
    {
        cerr << "set_edge_weight: " << target << " not a neighbor of " << id << endl;
    }
}

void Node::set_edge_weights(vector<struct NeighborWeight> new_neighbor_weights)
{
    for (struct NeighborWeight &p : new_neighbor_weights)
    {
        set_edge_weight(p.id, p.weight);
    }
}

LSA Node::to_lsa()
{
    LSA lsa = LSA(id, sequence_num);

    for (const int &id : neighbors)
    {
        lsa.add_weight(id, edge_weights[id]);
    }

    return lsa;
}

bool Node::from_lsa(LSA &lsa)
{
    if (lsa.sequence_num <= sequence_num)
        return false;

    sequence_num = lsa.sequence_num;
    set_neighbors(lsa.get_neighbors());
    set_edge_weights(lsa.weights);

    return true;
}

Graph::Graph(int self_id) : self_node{new Node(self_id)}, nodes{}
{
    nodes[self_node->id] = self_node;
}

Node *Graph::get_self_node()
{
    return self_node;
}

bool Graph::accept_lsa(LSA &lsa)
{
    bool is_fresh_lsa;

    if (!has_node(lsa.origin_id))
    {
        int id = lsa.origin_id;
        Node *new_node = new Node(id);
        is_fresh_lsa = new_node->from_lsa(lsa);
        nodes.insert(make_pair(id, new_node));
    }

    Node *target = nodes[lsa.origin_id];
    is_fresh_lsa = target->from_lsa(lsa);

    if (is_fresh_lsa)
        run_dijkstra();

    return is_fresh_lsa;
}

bool Graph::has_node(int target)
{
    return nodes.find(target) == nodes.end();
}
