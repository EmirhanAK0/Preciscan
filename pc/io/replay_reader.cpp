#include "replay_reader.h"
#include "../core/seq_metrics.h"
#include <fstream>

#ifdef _WIN32
  #include <winsock2.h> // ntohl
#else
  #include <arpa/inet.h>
#endif

static void set_error(std::string& e, const char* msg) { e = msg; }

bool replay_seq_file(
    const std::string& in_path,
    uint32_t target_count,
    SeqTracker& tracker,
    std::string& error
) {
    std::ifstream in(in_path, std::ios::binary);
    if (!in) { set_error(error, "failed to open input file"); return false; }

    uint32_t got = 0;
    while (true) {
        uint32_t net_seq = 0;
        in.read(reinterpret_cast<char*>(&net_seq), sizeof(net_seq));
        if (!in) break; // EOF or read error

        uint32_t seq = ntohl(net_seq);
        tracker.update(seq);
        got++;

        if (target_count != 0 && got >= target_count) break;
    }

    // If target_count requested but file ended early -> error
    if (target_count != 0 && got < target_count) {
        set_error(error, "input file ended before target_count");
        return false;
    }
    return true;
}
