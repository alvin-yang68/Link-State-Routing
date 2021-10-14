#pragma once

#include <utility>
#include <unordered_set>
#include <vector>

using namespace std;

class LSA
{
public:
    int origin_id;
    int sequence_num;
    vector<pair<int, int>> weights;
    LSA(int origin_id, int sequence_num);
    void add_weight(int node_id, int weight);
    unordered_set<int> get_neighbors();
};