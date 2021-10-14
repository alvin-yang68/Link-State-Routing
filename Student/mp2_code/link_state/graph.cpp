#include <stdexcept>

#include "graph.hpp"

using namespace std;

Node::Node(int id) : id{id}, sequence_num{-1}, neighbors{}, edge_weights{}
{
}

Node::Node(const Node &obj) : id{obj.id}, sequence_num{obj.sequence_num}, neighbors(obj.neighbors), edge_weights(obj.edge_weights)
{
}

bool Node::has_neighbor(int target)
{
    return neighbors.find(target) == neighbors.end();
}

int Node::get_edge_weight(int target)
{
    if (!has_neighbor(target))
        throw out_of_range("non-existent edge");

    try
    {
        return edge_weights.at(target);
    }
    catch (const out_of_range &error)
    {
        return 1;
    }
}

void Node::set_edge_weight(int target, int new_weight)
{
    edge_weights[target] = new_weight;
}

void Node::set_edge_weights(vector<pair<int, int>> new_weights)
{
    for (pair<int, int> &p : new_weights)
    {
        set_edge_weight(p.first, p.second);
    }
}

LSA Node::generate_lsa()
{
}

Graph::Graph(int self_id) : nodes{}, self_id{self_id}
{
}

void Graph::accept(LSA &lsa)
{
    if (!has_node(lsa.origin_id))
    {
        int key = lsa.origin_id;
        Node value = Node(lsa.origin_id);
        nodes.insert(make_pair(key, value));
    }

    Node target = nodes[lsa.origin_id];

    if (lsa.sequence_num <= target.sequence_num)
        return;

    target.sequence_num = lsa.sequence_num;
    target.set_neighbors(lsa.get_neighbors());
    target.set_edge_weights(lsa.weights);
}

bool Graph::has_node(int target)
{
    return nodes.find(target) == nodes.end();
}

Node *Graph::get_self_node()
{
    return &nodes[self_id];
}
