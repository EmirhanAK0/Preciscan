#include "udp_receiver.h"
#include "udp_socket.h"
#include "../core/seq_metrics.h"

bool udp_receive_seq_stream(
    int port,
    uint32_t target_count,
    SeqTracker& tracker,
    std::string& error
) {
    UdpSocket sock(port, error);
    if (!sock.is_open()) return false;

    uint32_t got = 0;
    while (got < target_count) {
        uint32_t net_seq = 0;
        int recvd = sock.recv(&net_seq, sizeof(net_seq));
        if (recvd == (int)sizeof(net_seq)) {
            tracker.update(ntohl(net_seq));
            got++;
        }
    }
    return true;
}
