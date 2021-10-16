#pragma once

#include <iostream>
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
    Node(const Node &obj);
    Node(int id);
    unordered_set<int> get_neighbors();
    void set_neighbors(const unordered_set<int> &new_neighbors);
    int get_edge_weight(int target);
    void set_edge_weight(int target, int new_weight);
    LSA to_lsa();
    bool from_lsa(LSA &lsa);

private:
    unordered_set<int> neighbors;
    unordered_map<int, int> edge_weights;
    bool has_neighbor(int target);
    void set_edge_weights(vector<struct NeighborWeight> new_weights);
};

/** Maintains the network topology, including information about the shortest paths **/
class Graph
{
public:
    Graph(int self_id);
    Node *get_self_node();
    bool accept_lsa(LSA &lsa);
    int find_next_hop(int destination_id);

private:
    Node *self_node;
    unordered_map<int, Node *> nodes;
    bool has_node(int target);
    void run_dijkstra();
};