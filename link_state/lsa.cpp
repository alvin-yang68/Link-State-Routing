#include <assert.h>

#include "lsa.hpp"

using namespace std;

LSA::LSA(uint16_t origin_id, uint32_t sequence_num) : origin_id{origin_id}, sequence_num{sequence_num}, edges{}
{
}

void LSA::add_edge(uint16_t node_id, uint32_t weight)
{
    struct EdgeToNeighbor edge;
    edge.neighbor_id = node_id;
    edge.weight = weight;

    edges.push_back(edge);
}

size_t LsaSerializer::serialize(const LSA &lsa, char *buffer, size_t buffer_len)
{
    size_t idx = 0;

    idx += add_short(lsa.origin_id, buffer + idx);
    idx += add_long(lsa.sequence_num, buffer + idx);

    for (const EdgeToNeighbor &edge : lsa.edges)
    {
        idx += add_short(edge.neighbor_id, buffer + idx);
        idx += add_long(edge.weight, buffer + idx);
    }

    buffer[idx] = '\0';
    return idx;
}

void LsaSerializer::deserialize(const char *buffer, size_t buffer_len, LSA *lsa)
{
    size_t idx = 0;

    lsa->origin_id = extract_short(buffer + idx);
    idx += sizeof(uint16_t);

    lsa->sequence_num = extract_long(buffer + idx);
    idx += sizeof(uint32_t);

    while (idx < buffer_len)
    {
        uint16_t neighbor_id = extract_short(buffer + idx);
        idx += sizeof(uint16_t);

        uint32_t edge_weight = extract_long(buffer + idx);
        idx += sizeof(uint32_t);

        lsa->add_edge(neighbor_id, edge_weight);
    }

    assert(idx == buffer_len);
}
