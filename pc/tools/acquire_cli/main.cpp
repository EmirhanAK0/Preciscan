#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <algorithm>

#include "seq_metrics.h"
#include "metrics_writer.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>


#endif

struct Config {
    // common
    std::string mode = "sim"; // sim | udp
    std::string metrics_out = "";


    // sim
    uint32_t count = 10000;
    double loss = 0.0;
    double ooo = 0.0;
    double dup = 0.0;

    // udp
    int port = 9000;
    uint32_t udp_count = 10000;
};

static void die(const std::string& msg) {
    std::cerr << msg << "\n";
    std::exit(2);
}

static bool parse_u32(const std::string& s, uint32_t& out) {
    try { out = static_cast<uint32_t>(std::stoul(s)); return true; }
    catch (...) { return false; }
}

static bool parse_f64(const std::string& s, double& out) {
    try { out = std::stod(s); return true; }
    catch (...) { return false; }
}

static Config parse_args(int argc, char** argv) {
    Config cfg;

    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];

        auto need = [&](const char* name) -> std::string {
            if (i + 1 >= argc) die(std::string("Missing value for ") + name);
            return std::string(argv[++i]);
            };

        if (a == "--mode") {
            cfg.mode = need("--mode");
            if (cfg.mode != "sim" && cfg.mode != "udp") die("Invalid --mode (use sim or udp)");
        }
        else if (a == "--count") {
            uint32_t v{};
            if (!parse_u32(need("--count"), v)) die("Invalid --count");
            cfg.count = v;
        }
        else if (a == "--loss") {
            double v{};
            if (!parse_f64(need("--loss"), v)) die("Invalid --loss");
            cfg.loss = v;
        }
        else if (a == "--ooo") {
            double v{};
            if (!parse_f64(need("--ooo"), v)) die("Invalid --ooo");
            cfg.ooo = v;
        }
        else if (a == "--dup") {
            double v{};
            if (!parse_f64(need("--dup"), v)) die("Invalid --dup");
            cfg.dup = v;
        }
        else if (a == "--port") {
            cfg.port = std::stoi(need("--port"));
        }
        else if (a == "--udp-count") {
            uint32_t v{};
            if (!parse_u32(need("--udp-count"), v)) die("Invalid --udp-count");
            cfg.udp_count = v;
        }
        else if (a == "--metrics-out") {
            cfg.metrics_out = need("--metrics-out");
        }

        else if (a == "--help" || a == "-h") {
            std::cout <<
                R"(acquire_cli (M2.2: sim + udp)

SIM mode:
  acquire_cli --mode sim --count 20000 --loss 0.001 --ooo 0.0005 --dup 0.0005

UDP mode (receiver):
  acquire_cli --mode udp --port 9000 --udp-count 20000

Notes:
- UDP payload for M2.2 is 4 bytes: uint32 seq in network byte order.
)";
            std::exit(0);
        }
        else {
            die(std::string("Unknown arg: ") + a);
        }
    }

    return cfg;
}

static std::vector<uint32_t> simulate_seq_stream(const Config& cfg) {
    std::mt19937 rng{ std::random_device{}() };
    std::uniform_real_distribution<double> uni(0.0, 1.0);

    uint32_t seq = 0;
    std::vector<uint32_t> out;
    out.reserve(static_cast<size_t>(cfg.count) * 2);

    for (uint32_t i = 0; i < cfg.count; i++) {
        seq += 1;

        // loss: create a gap by skipping one id
        if (uni(rng) < cfg.loss) {
            seq += 1;
        }

        out.push_back(seq);

        // duplicate
        if (uni(rng) < cfg.dup) {
            out.push_back(seq);
        }

        // out-of-order: swap last two
        if (out.size() >= 2 && uni(rng) < cfg.ooo) {
            std::swap(out[out.size() - 1], out[out.size() - 2]);
        }
    }

    return out;
}

static void run_udp_receiver(const Config& cfg, SeqTracker& tracker) {
#ifdef _WIN32
    WSADATA wsa{};
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) die("WSAStartup failed");
#endif

    int sock = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) die("socket() failed");

    sockaddr_in local{};
    local.sin_family = AF_INET;
    local.sin_port = htons((uint16_t)cfg.port);
    local.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (sockaddr*)&local, sizeof(local)) != 0) {
        die("bind() failed (port in use or blocked)");
    }

    uint32_t got = 0;
    while (got < cfg.udp_count) {
        uint32_t net_seq = 0;
        int recvd = recvfrom(sock, (char*)&net_seq, sizeof(net_seq), 0, nullptr, nullptr);
        if (recvd == (int)sizeof(net_seq)) {
            uint32_t seq = ntohl(net_seq);
            tracker.update(seq);
            got++;
        }
        // If recvd != 4, ignore (noise/other packets)
    }

#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif
}

int main(int argc, char** argv) {
    const auto cfg = parse_args(argc, argv);
    SeqTracker tracker;

    const auto t0 = std::chrono::steady_clock::now();

    if (cfg.mode == "sim") {
        auto stream = simulate_seq_stream(cfg);
        for (uint32_t s : stream) tracker.update(s);
    }
    else if (cfg.mode == "udp") {
        run_udp_receiver(cfg, tracker);
    }
    else {
        die("Invalid mode");
    }

    const auto t1 = std::chrono::steady_clock::now();
    const double dt = std::chrono::duration<double>(t1 - t0).count();

    const auto& st = tracker.stats;

    std::cout << "=== Seq Stats (M2.2 " << cfg.mode << ") ===\n";
    std::cout << "rx=" << st.rx
        << " missing=" << st.missing
        << " ooo=" << st.ooo
        << " dup=" << st.dup << "\n";
    std::cout << "PLR=" << tracker.plr() << "\n";
    std::cout << "time_s=" << dt << "\n";
    if (!cfg.metrics_out.empty()) {
        if (!write_metrics_json(cfg.metrics_out, cfg.mode, cfg.port, (cfg.mode == "udp" ? cfg.udp_count : cfg.count),
            tracker.stats, tracker.plr(), dt)) {
            std::cerr << "Failed to write metrics to: " << cfg.metrics_out << "\n";
            return 2;
        }
        std::cout << "Wrote metrics: " << cfg.metrics_out << "\n";
    }


    return 0;
}

