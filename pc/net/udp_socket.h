#pragma once
#include <string>
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

// RAII wrapper: constructor'da socket acar, destructor'da kapatir
// Boylece her fonksiyonda tekrarlanan WSAStartup/bind/closesocket bloklari ortadan kalkar
class UdpSocket {
public:
    // Basarili acilista is_open() true döner
    UdpSocket(int port, std::string& error) {
#ifdef _WIN32
        WSADATA wsa{};
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            error = "WSAStartup failed";
            return;
        }
        wsa_ok_ = true;
#endif
        sock_ = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock_ < 0) {
            error = "socket() failed";
            return;
        }

        sockaddr_in local{};
        local.sin_family = AF_INET;
        local.sin_port = htons((uint16_t)port);
        local.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(sock_, (sockaddr*)&local, sizeof(local)) != 0) {
            error = "bind() failed (port in use or blocked)";
            close_sock();
            return;
        }

        open_ = true;
    }

    ~UdpSocket() {
        close_sock();
    }

    bool is_open() const { return open_; }

    // Ham veri al, alinan byte sayisini dondur
    int recv(void* buf, int len) {
        return recvfrom(sock_, (char*)buf, len, 0, nullptr, nullptr);
    }

    // Kopyalamaya izin yok (RAII kurali)
    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;

private:
    int sock_ = -1;
    bool open_ = false;
#ifdef _WIN32
    bool wsa_ok_ = false;
#endif

    void close_sock() {
        if (sock_ >= 0) {
#ifdef _WIN32
            closesocket(sock_);
#else
            ::close(sock_);
#endif
            sock_ = -1;
        }
#ifdef _WIN32
        if (wsa_ok_) {
            WSACleanup();
            wsa_ok_ = false;
        }
#endif
        open_ = false;
    }
};
