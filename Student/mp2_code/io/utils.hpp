#pragma once

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

// Adds a short int (host order) to the buffer in network order
int add_short(short int num, char *buffer);

// Adds a long int (host order) to the buffer in network order
int add_long(long int num, char *buffer);

// Extracts a short int from network buffer and returns it in host order
short int extract_short(const char *buffer);

// Extracts a long int from network buffer and returns it in host order
long int extract_long(const char *buffer);