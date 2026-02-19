#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>
#include <fstream> // REPLAY modu için dosya okuma eklendi

// Platform baðýmsýz socket ayarlarý
#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")
using sock_t = SOCKET;
#define CLOSESOCKET(s) closesocket(s)
#else
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
using sock_t = int;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define CLOSESOCKET(s) close(s)
#endif

// Proje dosyalarý
#include "config.h"
#include "seq_metrics.h"
#include "spsc_ring_buffer.h"
#include "metrics_writer.h"

// --- CONSUMER THREAD (Veriyi Ýþleyen Ýþçi) ---
void consumer_worker(SPSCRingBuffer& queue, const Config& cfg, SeqTracker& tracker) {
    Packet pkt;
    while (queue.pop(pkt)) {
        // Ýlk 4 byte = uint32 seq
        if (pkt.data.size() >= 4) {
            uint32_t net_seq;
            std::memcpy(&net_seq, pkt.data.data(), 4);
            uint32_t seq = ntohl(net_seq); // Network -> Host order
            tracker.update(seq);
        }
    }
}

// --- MAIN (PRODUCER THREAD - ACQUISITION & REPLAY) ---
int main(int argc, char** argv) {
    Config cfg;
    try {
        cfg = Config::from_cli(argc, argv);
    }
    catch (const std::exception& e) {
        std::cerr << "Config Error: " << e.what() << "\n";
        return 1;
    }

    if (cfg.mode != "udp" && cfg.mode != "replay") {
        std::cerr << "HATA: ML6 surumu '--mode udp' veya '--mode replay' destekler.\n";
        return 1;
    }

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;
#endif

    // SPSC Lock-Free Queue ve Consumer Baþlatma
    SPSCRingBuffer queue(2000);
    SeqTracker tracker;

    std::cout << ">>> Acquisition START [Mod: " << cfg.mode << "]\n";
    std::thread consumer(consumer_worker, std::ref(queue), std::cref(cfg), std::ref(tracker));

    auto t0 = std::chrono::steady_clock::now();

    // ==========================================
    // VERÝ AKIÞI (UDP veya REPLAY DOSYASI)
    // ==========================================
    if (cfg.mode == "udp") {
        sock_t sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock == INVALID_SOCKET) return 1;

        int rcvbuf = 1024 * 1024; // 1MB
        setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&rcvbuf, (int)sizeof(rcvbuf));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(cfg.port);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            CLOSESOCKET(sock);
            return 1;
        }

#ifdef _WIN32
        DWORD timeout = 1000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, (int)sizeof(timeout));
#else
        struct timeval tv { 1, 0 };
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif

        std::vector<uint8_t> buffer(100000);
        sockaddr_in sender;
        int sender_len = sizeof(sender);

        bool running = true;
        while (running) {
#ifdef _WIN32
            int n = recvfrom(sock, (char*)buffer.data(), static_cast<int>(buffer.size()), 0, (sockaddr*)&sender, &sender_len);
#else
            socklen_t slen = sizeof(sender);
            int n = recvfrom(sock, (char*)buffer.data(), buffer.size(), 0, (sockaddr*)&sender, &slen);
#endif

            if (n > 0) {
                Packet pkt;
                pkt.data.assign(buffer.begin(), buffer.begin() + n);
                queue.try_push(std::move(pkt));
            }

            if (tracker.stats.rx >= static_cast<uint64_t>(cfg.udp_count)) running = false;
        }
        CLOSESOCKET(sock);
    }
    else if (cfg.mode == "replay") {
        if (cfg.replay_in.empty()) {
            std::cerr << "HATA: Replay modu icin --replay-in <dosya> belirtilmeli.\n";
            queue.stop();
            if (consumer.joinable()) consumer.join();
            return 1;
        }

        std::ifstream infile(cfg.replay_in, std::ios::binary);
        if (!infile) {
            std::cerr << "HATA: Replay dosyasi acilamadi: " << cfg.replay_in << "\n";
            queue.stop();
            if (consumer.joinable()) consumer.join();
            return 1;
        }

        uint32_t net_seq;
        // Dosyadan paketleri olabilecek maksimum hýzda oku (Stres Testi)
        while (infile.read(reinterpret_cast<char*>(&net_seq), sizeof(net_seq))) {
            Packet pkt;
            pkt.data.resize(4);
            std::memcpy(pkt.data.data(), &net_seq, 4);
            queue.try_push(std::move(pkt));
        }
    }

    // ==========================================
    // Kapanýþ ve Raporlama
    // ==========================================
    auto t1 = std::chrono::steady_clock::now();
    double dt = std::chrono::duration<double>(t1 - t0).count();

    queue.stop();
    if (consumer.joinable()) consumer.join();

#ifdef _WIN32
    WSACleanup();
#endif

    const auto& st = tracker.stats;
    std::cout << "\n=== SONUCLAR (ML6 SPSC CI Ready) ===\n";
    std::cout << "Mod: " << cfg.mode << "\n";
    std::cout << "Sure: " << dt << " s\n";
    std::cout << "RX (Alinan): " << st.rx << "\n";
    std::cout << "Missing: " << st.missing << "\n";
    std::cout << "Packet Loss Rate (PLR): " << tracker.plr() << "\n";
    std::cout << "-----------------------------\n";
    std::cout << "RING BUFFER DROPS: " << queue.get_drops() << "\n";
    std::cout << "-----------------------------\n";

    if (!cfg.metrics_out.empty()) {
        write_metrics_json(cfg.metrics_out, cfg.mode, cfg.port, cfg.udp_count, st, tracker.plr(), dt);
    }

    // CI OTOMASYONU ÝÇÝN KRÝTÝK KONTROL
    if (queue.get_drops() > 0 || tracker.plr() > 0.0) {
        std::cerr << "CI FAILED: Paket kaybi veya kuyruk tasmasi tespit edildi!\n";
        return 2; // HATA KODU
    }

    std::cout << "BASARILI: Sistem CI testini gecti.\n";
    return 0; // BAÞARI KODU
}