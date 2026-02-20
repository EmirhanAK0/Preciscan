#include <iostream>
#include <chrono>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

struct McuPacket {
    uint32_t seq;
    float y;
};

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    // [DUZELTME C4996] inet_addr deprecated, inet_pton kullanildi
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    uint32_t count = 0;
    float position = 0.0f;

    std::cout << ">>> GARANTI SIMULATOR BASLATILDI (8080) <<<\n";

    while (true) {
        McuPacket pkt = { count, position };
        sendto(sock, (const char*)&pkt, sizeof(pkt), 0, (sockaddr*)&addr, sizeof(addr));
        std::cout << "\r[SIM] Gonderilen -> Seq: " << count << " Y: " << position << " mm" << std::flush;
        count++;
        position += 0.5f;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    return 0;
}
