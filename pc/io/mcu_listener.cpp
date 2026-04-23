#include "mcu_listener.h"

#include <cstring>
#include <iostream>

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#pragma pack(push, 1)
struct McuTriggerEventPacket
{
    uint32_t session_id;
    uint32_t seq;
    uint32_t encoder_count;
    int32_t angle_mdeg;
    uint64_t tick_us;
};
#pragma pack(pop)

McuListener::McuListener(int port) : m_port(port), m_running(false)
{
}

McuListener::~McuListener()
{
    stop();
}

void McuListener::start()
{
    if (m_running) return;
    m_running = true;
    m_worker = std::thread(&McuListener::listenLoop, this);
}

void McuListener::stop()
{
    m_running = false;
    if (m_worker.joinable())
        m_worker.join();
}

bool McuListener::tryGetTriggerEvent(TriggerEvent& evt)
{
    std::lock_guard<std::mutex> lock(m_queueMutex);
    if (m_queue.empty()) return false;

    evt = m_queue.front();
    m_queue.pop();
    return true;
}

size_t McuListener::queueSize() const
{
    std::lock_guard<std::mutex> lock(m_queueMutex);
    return m_queue.size();
}

void McuListener::listenLoop()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        std::cerr << "[MCU] WSAStartup failed\n";
        return;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET)
    {
        std::cerr << "[MCU] Socket error: " << WSAGetLastError() << "\n";
        WSACleanup();
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons((u_short)m_port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        std::cerr << "[MCU] Bind error: " << WSAGetLastError() << "\n";
        closesocket(sock);
        WSACleanup();
        return;
    }

    std::cout << "[MCU] Listening on port " << m_port << "\n";

    char buffer[1024];

    while (m_running)
    {
        timeval tv{};
        tv.tv_sec = 0;
        tv.tv_usec = 100000;

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);

        if (select((int)sock + 1, &readfds, NULL, NULL, &tv) > 0)
        {
            int bytes = recv(sock, buffer, sizeof(buffer), 0);

            if (bytes >= (int)sizeof(McuTriggerEventPacket))
            {
                McuTriggerEventPacket pkt;
                std::memcpy(&pkt, buffer, sizeof(pkt));

                TriggerEvent evt{
                    pkt.session_id,
                    pkt.seq,
                    pkt.encoder_count,
                    pkt.angle_mdeg,
                    pkt.tick_us
                };

                {
                    std::lock_guard<std::mutex> lock(m_queueMutex);
                    m_queue.push(evt);
                }

                if (m_packetCb)
                    m_packetCb(evt);
            }
        }
    }

    closesocket(sock);
    WSACleanup();
}
