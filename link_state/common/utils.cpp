#include "utils.hpp"

int add_short(uint16_t num, char *buffer)
{
    uint16_t network_num = htons(num);
    memcpy(buffer, &network_num, sizeof(uint16_t));
    return sizeof(uint16_t);
}

int add_long(uint32_t num, char *buffer)
{
    uint32_t network_num = htonl(num);
    memcpy(buffer, &network_num, sizeof(uint32_t));
    return sizeof(uint32_t);
}

uint16_t extract_short(const char *buffer)
{
    uint16_t network_num;
    memcpy(&network_num, buffer, sizeof(uint16_t));
    return ntohs(network_num);
}

uint32_t extract_long(const char *buffer)
{
    uint32_t network_num;
    memcpy(&network_num, buffer, sizeof(uint32_t));
    return ntohl(network_num);
}

struct timespec generate_timespec_from_ms(long ms)
{
    long floor_sec = ms / 1000;
    long remainder_ms = ms - (floor_sec * 1000);

    struct timespec interval;
    interval.tv_sec = floor_sec;
    interval.tv_nsec = remainder_ms * 1000000;

    return interval;
}

bool has_prefix(const char *str, const char *prefix)
{
    return !strncmp(str, prefix, strlen(prefix));
}