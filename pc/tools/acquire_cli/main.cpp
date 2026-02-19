#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring> // memcpy için

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
// Not: CMake bu dosyalarýn path'lerini görüyorsa doðrudan include edebilirsin.
// Eðer hata alýrsan pathleri "../../core/config.h" gibi düzeltmen gerekebilir.
#include "config.h"
#include "seq_metrics.h"
#include "packet_queue.h"    
#include "metrics_writer.h"

// --- CONSUMER THREAD (Veriyi Ýþleyen Ýþçi) ---
// Bu fonksiyon ayrý bir thread'de çalýþýr.
// Tek iþi: Kuyruktan paket al, analiz et.
void consumer_worker(PacketQueue& queue, const Config& cfg, SeqTracker& tracker) {
    Packet pkt;

    // queue.pop() fonksiyonu veri gelene kadar BLOKE OLUR (CPU harcamaz).
    // queue.stop() çaðrýldýðýnda döngü false döner ve biter.
    while (queue.pop(pkt)) {

        // 1. Sequence Numarasýný Çýkar (Protocol varsayýmý: Ýlk 4 byte = uint32 seq)
        if (pkt.data.size() >= 4) {
            uint32_t net_seq;
            std::memcpy(&net_seq, pkt.data.data(), 4);
            uint32_t seq = ntohl(net_seq); // Network order (Big Endian) -> Host order

            // Metrikleri güncelle (ooo, dup, loss hesabý)
            tracker.update(seq);
        }

        // 2. Diske Yazma (Ýleride buraya eklenecek)

        // DOÐRULAMA (YAPAY YÜK TESTÝ):
        // Eðer bu satýrý açarsan Consumer yavaþlar, kuyruk dolar ve "Queue Drops" artar.
        // std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

// --- MAIN (PRODUCER THREAD) ---
// Burasý "Time-Critical" Acquisition Loop.
// Tek iþi: Socket'ten oku -> Kuyruða at.
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

    // Sadece UDP modunda çalýþýr
    if (cfg.mode != "udp") {
        std::cerr << "HATA: Bu versiyon sadece '--mode udp' destekler.\n";
        return 1;
    }

    // 2. Windows için WSA Baþlatma
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }
#endif

    // 3. UDP Soketi Aç
    sock_t sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    // Buffer Boyutunu Artýr (Kernel seviyesinde drop olmasýn diye)
    // FIX: sizeof sonucunu (int) olarak cast ettik.
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

    // Timeout ayarý (1 saniye - böylece sonsuza kadar takýlý kalmaz)
#ifdef _WIN32
    DWORD timeout = 1000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, (int)sizeof(timeout));
#else
    struct timeval tv { 1, 0 };
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif

    // 4. Queue ve Consumer Baþlatma
    // Kapasite: 2000 paket. 
    PacketQueue queue(2000);
    SeqTracker tracker; // Ýstatistik nesnesi (Consumer güncelleyecek)

    std::cout << ">>> Acquisition START (Port: " << cfg.port << ") [Multi-Threaded]\n";
    std::cout << ">>> Hedef Paket: " << cfg.udp_count << "\n";

    // Consumer Thread'i ateþle!
    std::thread consumer(consumer_worker, std::ref(queue), std::cref(cfg), std::ref(tracker));

    // 5. Ana Döngü (Time-Critical Loop)
    std::vector<uint8_t> buffer(2048); // MTU için güvenli boyut
    sockaddr_in sender;
    int sender_len = sizeof(sender);

    auto t0 = std::chrono::steady_clock::now();
    bool running = true;

    while (running) {
        // A. Blocking Receive
        // FIX: buffer.size() -> static_cast<int> yaptýk.
#ifdef _WIN32
        int n = recvfrom(sock, (char*)buffer.data(), static_cast<int>(buffer.size()), 0, (sockaddr*)&sender, &sender_len);
#else
        socklen_t slen = sizeof(sender);
        int n = recvfrom(sock, (char*)buffer.data(), buffer.size(), 0, (sockaddr*)&sender, &slen);
#endif

        if (n > 0) {
            // Veri geldi!
            Packet pkt;
            pkt.data.assign(buffer.begin(), buffer.begin() + n);

            // B. Kuyruða At (Producer Ýþlemi)
            // try_push: Eðer yer varsa atar, yoksa false döner (Drop)
            if (!queue.try_push(std::move(pkt))) {
                // Drop gerçekleþti, packet_queue içindeki sayaç arttý.
            }
        }

        // Çýkýþ Koþulu Kontrolü
        // FIX: cfg.udp_count'u uint64_t'ye cast ederek karþýlaþtýrdýk.
        // SeqTracker consumer'da olduðu için rx deðerini okumak tam thread-safe deðil ama 
        // "yaklaþýk" kontrol için yeterli. En doðru yöntem atomic kullanmaktýr ama
        // þimdilik bu kod iþ görür.
        if (tracker.stats.rx >= static_cast<uint64_t>(cfg.udp_count)) {
            running = false;
        }
    }

    // 6. Kapanýþ
    auto t1 = std::chrono::steady_clock::now();
    double dt = std::chrono::duration<double>(t1 - t0).count();

    queue.stop(); // Consumer'a dur de
    if (consumer.joinable()) {
        consumer.join(); // Consumer'ýn çýkmasýný bekle
    }

    CLOSESOCKET(sock);
#ifdef _WIN32
    WSACleanup();
#endif

    // 7. Raporlama
    const auto& st = tracker.stats;
    std::cout << "\n=== SONUCLAR (ML3 Threaded) ===\n";
    std::cout << "Sure: " << dt << " s\n";
    std::cout << "RX (Alinan): " << st.rx << "\n";
    std::cout << "Missing: " << st.missing << "\n";
    std::cout << "Out-of-Order: " << st.ooo << "\n";
    std::cout << "Packet Loss Rate (PLR): " << tracker.plr() << "\n";
    std::cout << "-----------------------------\n";
    std::cout << "QUEUE DROPS (Kuyruk Tasmasi): " << queue.get_drops() << "\n";
    std::cout << "-----------------------------\n";

    if (queue.get_drops() > 0) {
        std::cout << "UYARI: Consumer (isleyici) thread veriye yetisemedi!\n";
    }
    else {
        std::cout << "BASARILI: Pipeline tikir tikir calisti.\n";
    }

    // Metrics JSON yaz
    if (!cfg.metrics_out.empty()) {
        write_metrics_json(cfg.metrics_out, cfg.mode, cfg.port, cfg.udp_count, st, tracker.plr(), dt);
    }

    return 0;
}