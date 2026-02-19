#include "config.h"
#include "cli_args.h"
#include <stdexcept>

static uint32_t to_u32(const std::string& s, const std::string& key) {
    try { return static_cast<uint32_t>(std::stoul(s)); }
    catch (...) { throw std::runtime_error("Invalid " + key); }
}
static double to_f64(const std::string& s, const std::string& key) {
    try { return std::stod(s); }
    catch (...) { throw std::runtime_error("Invalid " + key); }
}
static int to_i32(const std::string& s, const std::string& key) {
    try { return std::stoi(s); }
    catch (...) { throw std::runtime_error("Invalid " + key); }
}

const char* Config::help_text() {
    return R"(acquire_cli

SIM mode:
  acquire_cli --mode sim --count 20000 --loss 0.001 --ooo 0.0005 --dup 0.0005 --metrics-out metrics_sim.json

UDP mode (receiver):
  acquire_cli --mode udp --port 9001 --udp-count 20000 --metrics-out metrics_udp.json
UDP mode (recording):
acquire_cli --mode replay --replay-in udp_raw.bin --metrics-out metrics_replay.json

Notes:
- UDP payload (M2.2) is 4 bytes: uint32 seq in network byte order.
)";
}

Config Config::from_cli(int argc, char** argv) {
    CliArgs cli(argc, argv);
    Config cfg;

    if (cli.has("--help") || cli.has("-h")) {
        // signal to caller by throwing a special message
        throw std::runtime_error("__HELP__");
    }

    if (cli.has("--mode")) cfg.mode = cli.value("--mode");
    if (cfg.mode != "sim" && cfg.mode != "udp" && cfg.mode != "replay")
        throw std::runtime_error("Invalid --mode (use sim or udp or replay)");


    if (cli.has("--metrics-out")) cfg.metrics_out = cli.value("--metrics-out");

    if (cli.has("--count")) cfg.count = to_u32(cli.value("--count"), "--count");
    if (cli.has("--loss"))  cfg.loss  = to_f64(cli.value("--loss"), "--loss");
    if (cli.has("--ooo"))   cfg.ooo   = to_f64(cli.value("--ooo"), "--ooo");
    if (cli.has("--dup"))   cfg.dup   = to_f64(cli.value("--dup"), "--dup");

    if (cli.has("--port")) cfg.port = to_i32(cli.value("--port"), "--port");
    if (cli.has("--udp-count")) cfg.udp_count = to_u32(cli.value("--udp-count"), "--udp-count");
    if (cli.has("--record-out")) cfg.record_out = cli.value("--record-out");
    if (cli.has("--replay-in")) cfg.replay_in = cli.value("--replay-in");
    if (cli.has("--run-dir")) cfg.run_dir = cli.value("--run-dir");




    return cfg;
}
