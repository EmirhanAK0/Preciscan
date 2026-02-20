#include "udp_recorder.h"
#include "udp_socket.h"
#include "../core/seq_metrics.h"
#include <fstream>

bool udp_record_seq_stream(
    int port,
    uint32_t target_count,
    const std::string& out_path,
    SeqTracker& tracker,
    std::string& error
) {
    std::ofstream out(out_path, std::ios::binary | std::ios::trunc);
    if (!out) { error = "failed to open output file"; return false; }

    UdpSocket sock(port, error);
    if (!sock.is_open()) return false;

    uint32_t got = 0;
    while (got < target_count) {
        uint32_t net_seq = 0;
        int recvd = sock.recv(&net_seq, sizeof(net_seq));
        if (recvd == (int)sizeof(net_seq)) {
            out.write(reinterpret_cast<const char*>(&net_seq), sizeof(net_seq));
            if (!out) { error = "failed while writing output file"; break; }
            tracker.update(ntohl(net_seq));
            got++;
        }
    }

    return got == target_count;
}
