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

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

static void die(const char* msg) {
    std::cerr << msg << "\n";
    std::exit(2);
}

int main(int argc, char** argv) {
    std::string host = "127.0.0.1";
    int port = 9001;
    uint32_t count = 20000;
    int delay_us = 0;

    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        auto need = [&]() -> std::string {
            if (i + 1 >= argc) die("Missing value");
            return std::string(argv[++i]);
        };

        if (a == "--host") host = need();
        else if (a == "--port") port = std::stoi(need());
        else if (a == "--count") count = (uint32_t)std::stoul(need());
        else if (a == "--delay-us") delay_us = std::stoi(need());
        else if (a == "--help" || a == "-h") {
            std::cout <<
R"(udp_sender_sim
  --host 127.0.0.1
  --port 9001
  --count 20000
  --delay-us 0
)";
            return 0;
        } else {
            die("Unknown arg");
        }
    }

#ifdef _WIN32
    WSADATA wsa{};
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) die("WSAStartup failed");
#endif

    int sock = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) die("socket() failed");

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
#ifdef _WIN32
    if (InetPtonA(AF_INET, host.c_str(), &addr.sin_addr) != 1) die("InetPtonA failed");
#else
    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1) die("inet_pton failed");
#endif

    for (uint32_t seq = 1; seq <= count; seq++) {
        uint32_t net_seq = htonl(seq);
        int sent = sendto(sock, (const char*)&net_seq, sizeof(net_seq), 0,
                          (sockaddr*)&addr, sizeof(addr));
        if (sent != (int)sizeof(net_seq)) die("sendto failed");

        if (delay_us > 0) std::this_thread::sleep_for(std::chrono::microseconds(delay_us));
    }

#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif

    std::cout << "Sent " << count << " packets to " << host << ":" << port << "\n";
    return 0;
}
