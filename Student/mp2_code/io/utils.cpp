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
    char num_str[sizeof(short int) + 1];
    memcpy(num_str, buffer, sizeof(short int));
    num_str[sizeof(short int)] = '\0';

    return ntohs(atoi(num_str));
}

long int extract_long(const char *buffer)
{
    char num_str[sizeof(long int) + 1];
    memcpy(num_str, buffer, sizeof(long int));
    num_str[sizeof(long int)] = '\0';

    return ntohl(atoi(num_str));
}