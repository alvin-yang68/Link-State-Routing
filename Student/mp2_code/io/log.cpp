#include "log.hpp"

using namespace std;

Log::Log(char *filename) : file_handler{fopen(filename, "wb")}
{
}

void Log::add_forward_entry(int destination_id, int next_hop_id, const char *message)
{
    char log_line[5000];
    int line_length;

    line_length = sprintf(log_line, "forward packet dest %d nexthop %d message %s\n", destination_id, next_hop_id, message);

    fwrite(log_line, sizeof(char), line_length, file_handler);
}

void Log::add_send_entry(int destination_id, int next_hop_id, const char *message)
{
    char log_line[5000];
    int line_length;

    line_length = sprintf(log_line, "sending packet dest %d nexthop %d message %s\n", destination_id, next_hop_id, message);

    fwrite(log_line, sizeof(char), line_length, file_handler);
}

void Log::add_receive_entry(const char *message)
{
    char log_line[5000];
    int line_length;

    line_length = sprintf(log_line, "receive packet message %s\n", message);

    fwrite(log_line, sizeof(char), line_length, file_handler);
}

void Log::add_unreachable_entry(int destination_id)
{
    char log_line[5000];
    int line_length;

    line_length = sprintf(log_line, "unreachable dest %d\n", destination_id);

    fwrite(log_line, sizeof(char), line_length, file_handler);
}