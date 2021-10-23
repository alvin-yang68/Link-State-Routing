#pragma once

#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <queue>

#include "link_state/types.hpp"
#include "link_state/lsa.hpp"

#define NUM_OF_NODES 256

using namespace std;

class Node
{
public:
    const int id;
    int sequence_num;
    unordered_set<int> neighbors;
    Node(const Node &obj);
    Node(int id);
    void insert_neighbor(int target);
    bool has_neighbor(int target);
    int get_edge_weight(int target);
    void set_edge_weight(int target, int new_weight);
    LSA generate_lsa();

private:
    unordered_map<int, int> edge_weights;
    void set_edge_weights(vector<struct EdgeToNeighbor> new_weights);
};

/** Maintains the network topology, including information about the shortest paths **/
class Graph
{
public:
    Node *get_node(int id);
    Node *register_node(Node *node);
    void set_edge_weight_pairs(int source_id, int target_id, int new_weight);
    bool accept_lsa(LSA &lsa);

private:
    unordered_map<int, Node *> nodes;
    bool has_node(int target);
};

class RouteFinder
{
public:
    RouteFinder(int self_id);
    void register_graph(Graph *graph);
    int find_next_hop(int destination_id);
    void run_dijkstra();

private:
    const int self_id;
    Graph *graph;
    vector<int> distances;
    vector<int> predecessors;
    priority_queue<EdgeToNeighbor, vector<EdgeToNeighbor>, CompareWeight> frontier;
    void reset_states();
};

template <class Q>
void clear_queue(Q &q)
{
    q = Q();
}