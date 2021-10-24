#include "log.hpp"

using namespace std;

Log::Log(const char *filename) : file_handler{fopen(filename, "wb")}
{
}

void Log::add_forward_entry(int destination_id, int next_hop_id, const char *message)
{
    char log_line[5000];
    int line_length;

    line_length = sprintf(log_line, "forward packet dest %d nexthop %d message %s\n", destination_id, next_hop_id, message);

    write_to_file(log_line, line_length);
}

void Log::write_to_file(const char *log_line, int line_length)
{
    fwrite(log_line, sizeof(char), line_length, file_handler);
    fflush(file_handler);
}

void Log::add_send_entry(int destination_id, int next_hop_id, const char *message)
{
    char log_line[5000];
    int line_length;

    line_length = sprintf(log_line, "sending packet dest %d nexthop %d message %s\n", destination_id, next_hop_id, message);

    write_to_file(log_line, line_length);
}

void Log::add_receive_entry(const char *message)
{
    char log_line[5000];
    int line_length;

    line_length = sprintf(log_line, "receive packet message %s\n", message);

    write_to_file(log_line, line_length);
}

void Log::add_unreachable_entry(int destination_id)
{
    char log_line[5000];
    int line_length;

    line_length = sprintf(log_line, "unreachable dest %d\n", destination_id);

    write_to_file(log_line, line_length);
}