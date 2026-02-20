#include "config.h"
#include <stdexcept>
#include <string>

Config Config::from_cli(int argc, char** argv) {
    Config cfg;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--mode" && i + 1 < argc)          cfg.mode       = argv[++i];
        else if (arg == "--replay-in"   && i + 1 < argc) cfg.replay_in  = argv[++i];
        else if (arg == "--metrics-out" && i + 1 < argc) cfg.metrics_out= argv[++i];
        else if (arg == "--udp-count"   && i + 1 < argc) cfg.udp_count  = std::stoi(argv[++i]);
        else if (arg == "--port"        && i + 1 < argc) cfg.port       = std::stoi(argv[++i]);
    }

    if (cfg.mode != "sim"   &&
        cfg.mode != "udp"   &&
        cfg.mode != "replay"&&
        cfg.mode != "laser" &&
        cfg.mode != "mcu") {
        throw std::runtime_error("Invalid --mode (use sim, udp, replay, laser or mcu)");
    }
    return cfg;
}
