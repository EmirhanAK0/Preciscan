#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>
#include <fstream>

// Micro-Epsilon SDK ve Yeni Paket Mimarisi
#include "InterfaceLLT_2.h"
#include "packet_v2.h"

// Mevcut Altyapi
#include "config.h"
#include "seq_metrics.h"
#include "spsc_ring_buffer.h"

// --- LAZER CALLBACK FONKSIYONU (Asekron Kesme) ---
// Bu fonksiyon LLT.dll tarafindan arka planda yuksek oncelikli bir thread icinde cagrilir.
void _stdcall LaserCallback(const unsigned char* pData, unsigned int nSize, void* pUserData) {
    // Gelen ham veriyi (pData) daha sonra ConvertProfile2Values ile gercek mm cinsinden
    // X ve Z koordinatlarina (LaserProfilePacket) donusturecegiz.
    // Simdilik sadece verinin geldigini kanitliyoruz!
    std::cout << "\r[LAZER] Yeni profil alindi! Boyut: " << nSize << " byte" << std::flush;
}

// --- CONSUMER THREAD (Veriyi Isleyen Isci - Replay/UDP icin) ---
void consumer_worker(SPSCRingBuffer& queue, const Config& cfg, SeqTracker& tracker) {
    Packet pkt;
    while (queue.pop(pkt)) {
        if (pkt.data.size() >= 4) {
            uint32_t net_seq;
            std::memcpy(&net_seq, pkt.data.data(), 4);
            uint32_t seq = ntohl(net_seq); 
            tracker.update(seq);
        }
    }
}

int main(int argc, char** argv) {
    Config cfg;
    try { cfg = Config::from_cli(argc, argv); }
    catch (const std::exception& e) { std::cerr << "Config Error: " << e.what() << "\n"; return 1; }

    SPSCRingBuffer queue(100000); 
    SeqTracker tracker; 

    std::cout << ">>> Acquisition START [Mod: " << cfg.mode << "]\n";

    // ==========================================
    // YENI ROTA: GERCEK DONANIM (LAZER) MODU
    // ==========================================
    if (cfg.mode == "laser") {
        std::cout << ">>> Micro-Epsilon SDK Yukleniyor...\n";
        bool bLoadError = false;
        
        // LLT.dll dosyasini arar ve RAM'e yukler
        CInterfaceLLT llt("LLT.dll", &bLoadError);
        
        if (bLoadError) {
            std::cerr << "HATA: LLT.dll bulunamadi! Lutfen LLT.dll dosyasini acquire_cli.exe ile ayni klasore koyun.\n";
            return 1;
        }

        std::cout << ">>> SDK Basariyla yuklendi! Cihaz araniyor (Ethernet)...\n";
        
        // Arayuz tipini Ethernet (GigE Vision) olarak ayarla
        llt.SetDeviceInterface(INTF_TYPE_ETHERNET, 0);

        // Lazere Baglan
        int conn_res = llt.Connect();
        if (conn_res < 1) {
            std::cerr << "HATA: Lazere baglanilamadi! (Hata Kodu: " << conn_res << ")\n";
            std::cerr << "Cihazin acik, ethernet kablosunun takili ve IP adresinin dogru oldugundan emin olun.\n";
            return 1;
        }

        std::cout << ">>> BINGO! Lazer Baglantisi Kuruldu!\n";

        // Cihaz Bilgilerini Al
        char devName[100], venName[100];
        llt.GetDeviceName(devName, 100, venName, 100);
        std::cout << "Cihaz: " << venName << " " << devName << "\n";

        // Callback Kaydi (Veri geldiginde LaserCallback fonksiyonunu cagir)
        llt.RegisterCallback(STD_CALL, (void*)LaserCallback, nullptr);

        // Veri Akisini Baslat
        llt.TransferProfiles(NORMAL_TRANSFER, 1);

        std::cout << ">>> Lazer Veri Akisi Basladi! (Durdurmak icin CTRL+C basin)\n";
        
        // Ana thread'i canli tut
        while(true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    // ==========================================
    // ESKI ROTALAR (CI Kalite Kontrolu Icin Korunuyor)
    // ==========================================
    else if (cfg.mode == "replay") {
        std::thread consumer(consumer_worker, std::ref(queue), std::cref(cfg), std::ref(tracker));
        std::ifstream infile(cfg.replay_in, std::ios::binary);
        if (infile) {
            uint32_t net_seq;
            while (infile.read(reinterpret_cast<char*>(&net_seq), sizeof(net_seq))) {
                Packet pkt; pkt.data.resize(4);
                std::memcpy(pkt.data.data(), &net_seq, 4);
                queue.try_push(std::move(pkt));
            }
        }
        queue.stop();
        if (consumer.joinable()) consumer.join();
        
        if (queue.get_drops() > 0 || tracker.plr() > 0.0) return 2; // CI Fail
    }
    else {
        std::cerr << "Mod desteklenmiyor veya UDP moduna henuz tam gecilmedi.\n";
    }

    return 0;
}
