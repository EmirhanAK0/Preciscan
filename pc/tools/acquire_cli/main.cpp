#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>

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
#include "spsc_ring_buffer.h"  // YENÝ: Lock-Free Buffer
#include "metrics_writer.h"

// --- CONSUMER THREAD (Veriyi Ýþleyen Ýþçi) ---
// Lock-free buffer'dan veri çeker.
void consumer_worker(SPSCRingBuffer& queue, const Config& cfg, SeqTracker& tracker) {
    Packet pkt;

    // pop() fonksiyonu veri gelene kadar döner (spin-wait/yield yapar).
    // queue.stop() çaðrýldýðýnda ve kuyruk boþaldýðýnda false döner.
    while (queue.pop(pkt)) {

        // 1. Sequence Numarasýný Çýkar (Ýlk 4 byte = uint32 seq)
        if (pkt.data.size() >= 4) {
            uint32_t net_seq;
            std::memcpy(&net_seq, pkt.data.data(), 4);
            uint32_t seq = ntohl(net_seq); // Network -> Host order

            // Metrikleri güncelle (ooo, dup, loss)
            tracker.update(seq);
        }

        // DOÐRULAMA (YAPAY YÜK TESTÝ):
        // Lock-free yapýnýn drop mekanizmasýný test etmek için burayý açabilirsin.
        // std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

// --- MAIN (PRODUCER THREAD - ACQUISITION) ---
// Sadece Socket'ten oku -> Lock-Free Kuyruða at. Asla bloke olmaz!
int main(int argc, char** argv) {
    // 1. Config Yükle
    Config cfg;
    try {
        cfg = Config::from_cli(argc, argv);
    }
    catch (const std::exception& e) {
        std::cerr << "Config Error: " << e.what() << "\n";
        return 1;
    }

    if (cfg.mode != "udp") {
        std::cerr << "HATA: ML4 sürümü sadece '--mode udp' destekler.\n";
        return 1;
    }

    // 2. Windows WSA Baþlatma
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }
#endif

    // 3. UDP Soketi Aç
    sock_t sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) return 1;

    // Kernel Receive Buffer'ý büyüt (Ýþletim sistemi paket düþürmesin)
    int rcvbuf = 1024 * 1024; // 1MB
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&rcvbuf, (int)sizeof(rcvbuf));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(cfg.port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "Bind error on port " << cfg.port << "\n";
        CLOSESOCKET(sock);
        return 1;
    }

    // Timeout (1 saniye)
#ifdef _WIN32
    DWORD timeout = 1000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, (int)sizeof(timeout));
#else
    struct timeval tv { 1, 0 };
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif

    // 4. SPSC Lock-Free Queue ve Consumer Baþlatma
    // Kapasite: 2000 paket. Allocation yapmaz, baþtan hafýzayý ayýrýr.
    SPSCRingBuffer queue(2000);
    SeqTracker tracker;

    std::cout << ">>> Acquisition START (Port: " << cfg.port << ") [ML4 Lock-Free SPSC]\n";
    std::cout << ">>> Hedef Paket: " << cfg.udp_count << "\n";

    // Consumer Thread Baþlat
    std::thread consumer(consumer_worker, std::ref(queue), std::cref(cfg), std::ref(tracker));

    // 5. Time-Critical Acquisition Loop
    std::vector<uint8_t> buffer(2048);
    sockaddr_in sender;
    int sender_len = sizeof(sender);

    auto t0 = std::chrono::steady_clock::now();
    bool running = true;

    while (running) {
        // A. Ýþletim Sisteminden Oku
#ifdef _WIN32
        int n = recvfrom(sock, (char*)buffer.data(), static_cast<int>(buffer.size()), 0, (sockaddr*)&sender, &sender_len);
#else
        socklen_t slen = sizeof(sender);
        int n = recvfrom(sock, (char*)buffer.data(), buffer.size(), 0, (sockaddr*)&sender, &slen);
#endif

        if (n > 0) {
            Packet pkt;
            pkt.data.assign(buffer.begin(), buffer.begin() + n);

            // B. Kuyruða At (Producer)
            // Kilit yok! try_push anýnda döner. Doluysa false (drop) döner.
            queue.try_push(std::move(pkt));
        }

        // Çýkýþ Koþulu Kontrolü
        if (tracker.stats.rx >= static_cast<uint64_t>(cfg.udp_count)) {
            running = false;
        }
    }

    // 6. Kapanýþ ve Temizlik
    auto t1 = std::chrono::steady_clock::now();
    double dt = std::chrono::duration<double>(t1 - t0).count();

    queue.stop(); // Consumer'a döngüyü kýr emri ver
    if (consumer.joinable()) {
        consumer.join();
    }

    CLOSESOCKET(sock);
#ifdef _WIN32
    WSACleanup();
#endif

    // 7. Raporlama
    const auto& st = tracker.stats;
    std::cout << "\n=== SONUCLAR (ML4 Lock-Free SPSC) ===\n";
    std::cout << "Sure: " << dt << " s\n";
    std::cout << "RX (Alinan): " << st.rx << "\n";
    std::cout << "Missing: " << st.missing << "\n";
    std::cout << "Out-of-Order: " << st.ooo << "\n";
    std::cout << "Packet Loss Rate (PLR): " << tracker.plr() << "\n";
    std::cout << "-----------------------------\n";
    std::cout << "RING BUFFER DROPS: " << queue.get_drops() << "\n";
    std::cout << "-----------------------------\n";

    if (queue.get_drops() > 0) {
        std::cout << "UYARI: Consumer yetisemedi, " << queue.get_drops() << " paket drop edildi (Drop-Newest).\n";
    }
    else {
        std::cout << "BASARILI: SPSC Ring Buffer pürüzsüz calisti.\n";
    }

    if (!cfg.metrics_out.empty()) {
        write_metrics_json(cfg.metrics_out, cfg.mode, cfg.port, cfg.udp_count, st, tracker.plr(), dt);
    }

    return 0;
}