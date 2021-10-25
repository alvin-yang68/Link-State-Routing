#pragma once

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

// Adds a short int (host order) to the buffer in network order
int add_short(uint16_t num, char *buffer);

// Adds a long int (host order) to the buffer in network order
int add_long(uint32_t num, char *buffer);

// Extracts a short int from network buffer and returns it in host order
uint16_t extract_short(const char *buffer);

// Extracts a long int from network buffer and returns it in host order
uint32_t extract_long(const char *buffer);