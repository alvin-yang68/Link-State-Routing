#pragma once

#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "types.hpp"
#include "lsa.hpp"

using namespace std;

class Node
{
public:
    const int id;
    int sequence_num;
    Node(int id);
    Node(const Node &obj);
    bool has_neighbor(int target);
    unordered_set<int> get_neighbors();
    void set_neighbors(const unordered_set<int> &new_neighbors);
    int get_edge_weight(int target);
    void set_edge_weight(int target, int new_weight);
    void set_edge_weights(vector<struct NeighborWeight> new_weights);
    LSA generate_lsa();

private:
    unordered_set<int> neighbors;
    unordered_map<int, int> edge_weights;
};

class Graph
{
public:
    Graph(int self_id);
    ~Graph();
    void accept_lsa(LSA &lsa);
    Node *get_self_node();

private:
    int self_id;
    unordered_map<int, Node> nodes;
    bool has_node(int target);
};