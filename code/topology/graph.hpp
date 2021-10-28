#pragma once

#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "common/types.hpp"
#include "lsa.hpp"
#include "common/config.hpp"

using namespace std;

class Node
{
public:
    uint32_t sequence_num;
    unordered_set<uint16_t> neighbors;
    Node() = default;
    Node(uint16_t id); // copy and move constructors, copy and move assignment operators, and destructor are implicit
    uint16_t get_id() const;
    uint32_t get_edge_weight(uint16_t target);
    void set_edge_weight(uint16_t target, uint32_t new_weight);
    LSA generate_lsa();
    void from_lsa(LSA &lsa);

private:
    uint16_t id;
    unordered_map<uint16_t, uint32_t> edge_weights;
    bool has_edge_weight(uint16_t target);
};

class Graph
{
public:
    Graph();
    Node *get_node(uint16_t id);
    void insert_node(Node *new_node);
    bool accept_lsa(LSA &lsa);

private:
    unordered_map<uint16_t, Node *> nodes;
    bool has_node(uint16_t target) const;
};
