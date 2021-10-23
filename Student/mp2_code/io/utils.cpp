#include "utils.hpp"

int add_short(short int num, char *buffer)
{
    short int network_num = htons(num);
    memcpy(buffer, &network_num, sizeof(short int));
    return sizeof(short int);
}

int add_long(long int num, char *buffer)
{
    long int network_num = htonl(num);
    memcpy(buffer, &network_num, sizeof(long int));
    return sizeof(long int);
}

short int extract_short(const char *buffer)
{
    short int network_num;
    memcpy(&network_num, buffer, sizeof(short int));
    return ntohs(network_num);
}

long int extract_long(const char *buffer)
{
    long int network_num;
    memcpy(&network_num, buffer, sizeof(long int));
    return ntohl(network_num);
}