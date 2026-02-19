#include "udp_receiver.h"
#include "../core/seq_metrics.h"

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

static void set_error(std::string& e, const char* msg) {
    e = msg;
}

bool udp_receive_seq_stream(
    int port,
    uint32_t target_count,
    SeqTracker& tracker,
    std::string& error
) {
#ifdef _WIN32
    WSADATA wsa{};
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        set_error(error, "WSAStartup failed");
        return false;
    }
#endif

    int sock = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        set_error(error, "socket() failed");
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }

    sockaddr_in local{};
    local.sin_family = AF_INET;
    local.sin_port = htons((uint16_t)port);
    local.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (sockaddr*)&local, sizeof(local)) != 0) {
        set_error(error, "bind() failed (port in use or blocked)");
#ifdef _WIN32
        closesocket(sock);
        WSACleanup();
#else
        close(sock);
#endif
        return false;
    }

    uint32_t got = 0;
    while (got < target_count) {
        uint32_t net_seq = 0;
        int recvd = recvfrom(sock, (char*)&net_seq, sizeof(net_seq), 0, nullptr, nullptr);
        if (recvd == (int)sizeof(net_seq)) {
            uint32_t seq =
#ifdef _WIN32
                ntohl(net_seq);
#else
                ntohl(net_seq);
#endif
            tracker.update(seq);
            got++;
        }
        // other sizes ignored
    }

#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif

    return true;
}
