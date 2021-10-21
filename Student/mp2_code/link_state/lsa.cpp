#include "lsa.hpp"

using namespace std;

LSA::LSA(int origin_id, int sequence_num) : origin_id{origin_id}, sequence_num{sequence_num}, edges{}
{
}

void LSA::add_weight(int node_id, int weight)
{
    struct EdgeToNeighbor edge;
    edge.neighbor_id = node_id;
    edge.weight = weight;

    edges.push_back(edge);
}

unordered_set<int> LSA::get_neighbors() const
{
    unordered_set<int> ids;
    ids.reserve(edges.size());

    for (const struct EdgeToNeighbor &edge : edges)
    {
        ids.insert(edge.neighbor_id);
    }

    return ids;
}