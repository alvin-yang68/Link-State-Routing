#pragma once

#include <unordered_set>
#include <vector>
#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>

#include "link_state/types.hpp"

using namespace std;

class LSA
{
public:
    int origin_id;
    int sequence_num;
    vector<struct EdgeToNeighbor> edges;
    LSA() = default;
    LSA(int origin_id, int sequence_num);
    void add_weight(int node_id, int weight);
    unordered_set<int> get_neighbors() const;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar &origin_id;
        ar &sequence_num;
        ar &edges;
    }
};