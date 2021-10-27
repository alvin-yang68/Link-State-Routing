#pragma once

#include <iostream>
#include <mutex>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <queue>

#include "link_state/types.hpp"
#include "link_state/lsa.hpp"

#include "common/config.hpp"

using namespace std;

class Node
{
public:
    int sequence_num;
    unordered_set<int> neighbors;
    Node() = default;
    Node(int id); // copy and move constructors, copy and move assignment operators, and destructor are implicit
    int get_id();
    int get_edge_weight(int target);
    void set_edge_weight(int target, int new_weight);
    LSA generate_lsa();
    void from_lsa(LSA &lsa);

private:
    int id;
    unordered_map<int, int> edge_weights;
    void init_node(int max_size);
    bool has_edge_weight(int target);
};

/** Maintains the network topology, including information about the shortest paths **/
class Graph
{
public:
    Graph();
    Graph(const Graph &other);      // copy constructor
    Graph(Graph &&other);           // move constructor
    Graph &operator=(Graph &other); // copy assignment operator
    Node get_node(int id);
    void set_node(Node &target);
    bool accept_lsa(LSA &lsa);

private:
    unordered_map<int, Node> nodes;
    mutex graph_mutex;
    unordered_map<int, Node> get_nodes_map_copy();
    Node &get_node_(int id);
    bool has_node(int target) const;
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
    Graph graph_snapshot;
    vector<int> distances;
    vector<vector<int>> predecessors;
    priority_queue<EdgeToNeighbor, vector<EdgeToNeighbor>, CompareWeight> frontier;
    void ready_states();
};

template <class Q>
void clear_queue(Q &q)
{
    q = Q();
}