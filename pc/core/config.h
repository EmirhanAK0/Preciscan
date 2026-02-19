#pragma once
#include <cstdint>
#include <string>

struct Config {
    std::string mode = "sim";      // sim | udp
    std::string metrics_out = "";  // optional output path
    std::string record_out = ""; // optional raw UDP dump path
    std::string replay_in = ""; // path to raw file
    std::string run_dir = "";


    // sim
    uint32_t count = 10000;
    double loss = 0.0;
    double ooo = 0.0;
    double dup = 0.0;

    // udp
    int port = 9001;
    uint32_t udp_count = 10000;

    static Config from_cli(int argc, char** argv); // throws on error
    static const char* help_text();
};
