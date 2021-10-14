#pragma once

#include <unordered_set>
#include <vector>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>

#include "types.hpp"

using namespace std;

class LSA
{
public:
    int origin_id;
    int sequence_num;
    vector<struct NeighborWeight> weights;
    LSA();
    LSA(int origin_id, int sequence_num);
    void add_weight(int node_id, int weight);
    unordered_set<int> get_neighbors();
};