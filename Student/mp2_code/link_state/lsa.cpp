#include "lsa.hpp"

using namespace std;

LSA::LSA(int origin_id, int sequence_num) : origin_id{origin_id}, sequence_num{sequence_num}, weights{}
{
}

void LSA::add_weight(int node_id, int weight)
{
    struct NeighborWeight pair = {
        .id = node_id,
        .weight = weight,
    };
    weights.push_back(pair);
}

unordered_set<int> LSA::get_neighbors()
{
    unordered_set<int> ids;
    ids.reserve(weights.size());

    for (struct NeighborWeight &pair : weights)
    {
        ids.insert(pair.id);
    }

    return ids;
}