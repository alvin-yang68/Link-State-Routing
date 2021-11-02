#pragma once

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

// Adds a short int (host order) to the string buffer in network order
int encode_short_into_str(char *buffer, uint16_t num);

// Adds a long int (host order) to the string buffer in network order
int encode_long_into_str(char *buffer, uint32_t num);

// Extracts a short int from network buffer and returns it in host order
uint16_t decode_short_from_str(const char *buffer);

// Extracts a long int from network buffer and returns it in host order
uint32_t decode_long_from_str(const char *buffer);

struct timespec generate_timespec_from_ms(long ms);

bool has_prefix(const char *str, const char *prefix);