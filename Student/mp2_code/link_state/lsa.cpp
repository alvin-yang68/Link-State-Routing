#include "lsa.hpp"

using namespace std;

LSA::LSA(int origin_id, int sequence_num) : origin_id{origin_id}, sequence_num{sequence_num}, weights{}
{
}

void LSA::add_weight(int node_id, int weight)
{
    weights.push_back(make_pair(node_id, weight));
}

unordered_set<int> LSA::get_neighbors()
{
    unordered_set<int> ids;
    ids.reserve(weights.size());

    for (pair<int, int> &weight : weights)
    {
        ids.insert(weight.first);
    }

    return ids;
}