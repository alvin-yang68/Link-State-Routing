#pragma once

#include <iostream>
#include <fstream>

using namespace std;

class Log
{
public:
    Log(const char *filename);
    void add_forward_entry(uint16_t destination_id, uint16_t next_hop_id, const char *message);
    void add_send_entry(uint16_t destination_id, uint16_t next_hop_id, const char *message);
    void add_receive_entry(const char *message);
    void add_unreachable_entry(uint16_t destination_id);

private:
    FILE *file_handler;
    void write_to_file(const char *log_line, size_t line_length);
};