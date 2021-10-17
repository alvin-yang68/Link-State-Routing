#pragma once

#include <iostream>
#include <fstream>

using namespace std;

class Log
{
public:
    Log(char *filename);
    void add_forward_entry(int destination_id, int next_hop_id, const char *message);
    void add_send_entry(int destination_id, int next_hop_id, const char *message);
    void add_receive_entry(const char *message);
    void add_unreachable_entry(int destination_id);

private:
    FILE *file_handler;
};

// template <typename... Args>
// string string_format(const string &format, Args... args)
// {
//     int size_s = snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
//     if (size_s <= 0)
//     {
//         throw runtime_error("Error during formatting.");
//     }
//     auto size = static_cast<size_t>(size_s);
//     auto buf = make_unique<char[]>(size);
//     snprintf(buf.get(), size, format.c_str(), args...);
//     return string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
// }