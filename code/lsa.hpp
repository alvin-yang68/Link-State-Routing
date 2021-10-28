#pragma once

#include <unordered_set>
#include <vector>

#include "common/types.hpp"
#include "common/utils.hpp"

using namespace std;

class LSA
{
public:
    uint16_t origin_id;
    uint32_t sequence_num;
    vector<struct EdgeToNeighbor> edges;
    LSA() = default;
    LSA(uint16_t origin_id, uint32_t sequence_num);
    void add_edge(uint16_t destination_id, uint32_t weight);
};

class LsaSerializer
{
public:
    size_t serialize(const LSA &lsa, char *buffer, size_t buffer_len);
    void deserialize(const char *buffer, size_t buffer_len, LSA *lsa);
};