#pragma once

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/access.hpp>

struct EdgeToNeighbor
{
public:
    int neighbor_id;
    int weight;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar &neighbor_id;
        ar &weight;
    }
};

struct CompareWeight
{
    constexpr bool operator()(EdgeToNeighbor const &n1, EdgeToNeighbor const &n2) const
    {
        return n1.weight > n2.weight;
    }
};