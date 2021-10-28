#pragma once

struct EdgeToNeighbor
{
public:
    uint16_t neighbor_id;
    uint32_t weight;
};

struct CompareWeight
{
    constexpr bool operator()(EdgeToNeighbor const &n1, EdgeToNeighbor const &n2) const
    {
        return n1.weight > n2.weight;
    }
};