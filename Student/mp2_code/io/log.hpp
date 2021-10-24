#pragma once

#include <iostream>
#include <fstream>

using namespace std;

class Log
{
public:
    Log(const char *filename);
    void add_forward_entry(int destination_id, int next_hop_id, const char *message);
    void add_send_entry(int destination_id, int next_hop_id, const char *message);
    void add_receive_entry(const char *message);
    void add_unreachable_entry(int destination_id);

private:
    FILE *file_handler;
    void write_to_file(const char *log_line, int line_length);
};