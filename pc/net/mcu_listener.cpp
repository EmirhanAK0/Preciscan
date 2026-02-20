#include "mcu_listener.h"
#include <iostream>
#include <winsock2.h>
#include <cstring>
#pragma comment(lib, "ws2_32.lib")

#pragma pack(push, 1)
struct McuFixedPacket {
    uint32_t trigger_seq;
    float y_position;
};
#pragma pack(pop)

McuListener::McuListener(int port) : m_port(port), m_running(false) {}

McuListener::~McuListener() {
    stop();
}

void McuListener::start() {
    if (m_running) return;
    m_running = true;
    m_worker = std::thread(&McuListener::listenLoop, this);
}

void McuListener::stop() {
    m_running = false;
    if (m_worker.joinable()) {
        m_worker.join();
    }
}

void McuListener::listenLoop() {
    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "[MCU] WSAStartup basarisiz!" << std::endl;
        return;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sock == INVALID_SOCKET) {
        std::cerr << "[MCU] Socket olusturulamadi! Hata: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    // [DUZELTME C4244] int -> u_short explicit cast
    addr.sin_port = htons(static_cast<u_short>(m_port));
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "[MCU] Port baglama hatasi! Hata: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return;
    }

    std::cout << "[MCU] Dinleme basladi: " << m_port << std::endl;

    char buffer[1024];
    while (m_running) {
        timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000;

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);

        // [DUZELTME C4244] SOCKET -> int explicit cast (Windows select() gereksinimi)
        if (select(static_cast<int>(sock) + 1, &readfds, NULL, NULL, &tv) > 0) {
            int bytes = recv(sock, buffer, 1024, 0);
            if (bytes >= (int)sizeof(McuFixedPacket)) {
                McuFixedPacket pkt;
                std::memcpy(&pkt, buffer, sizeof(McuFixedPacket));
                std::cout << "\r[MCU] Tetik: " << pkt.trigger_seq
                          << " Y: " << pkt.y_position << " mm    " << std::flush;
            }
        }
    }

    closesocket(sock);
    WSACleanup();
}
