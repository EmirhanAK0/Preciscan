#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include "mcu_listener.h"
#include "laser_manager.h"
#include "spsc_ring_buffer.h"
#include "ring_buffer_sink.h"
#include "../io/disk_writer.h"
#include "config.h"

int main(int argc, char** argv) {
    Config cfg;
    try { cfg = Config::from_cli(argc, argv); }
    catch (const std::exception& e) {
        std::cerr << "Hata: " << e.what() << "\n";
        return 1;
    }

    std::cout << "========================================\n";
    std::cout << "       PRECISCAN CORE v2.1 (PIPELINE)  \n";
    std::cout << "========================================\n";
    std::cout << "Mod: " << cfg.mode << " | Port: " << cfg.port << "\n\n";

    if (cfg.mode == "mcu") {
        // Sadece MCU listener -- lazer olmadan test icin
        McuListener arduino(cfg.port);
        arduino.start();

        std::cout << "[ANA] MCU dinleniyor (port " << cfg.port << ").\n";
        std::cout << "[ANA] Durdurmak icin Enter'a basin...\n";
        std::cin.get();

        std::cout << "[ANA] Durduruluyor...\n";
        arduino.stop();
        std::cout << "[ANA] Temiz kapandi.\n";
    }
    else if (cfg.mode == "laser") {
        // --- PIPELINE KURULUMU ---
        // 1. Ring buffer: 256 profil kapasiteli
        SPSCRingBuffer ring(256);

        // 2. Sink: callback thread producer, consumer thread tuketici
        RingBufferSink sink(ring);

        // 3. MCU listener (trigger/pozisyon verisi)
        McuListener arduino(cfg.port);
        arduino.start();

        // 4. Lazer manager: sink bagliyor
        LaserManager laser("LLT.dll", &sink);
        if (laser.init()) {
            if (laser.connect()) {
                std::cout << ">>> LAZER BAGLANTISI BASARILI.\n";
                laser.startAcquisition();
            } else {
                std::cout << ">>> UYARI: Lazer bulunamadi, sadece MCU dinleniyor.\n";
            }
        }

        // 5. Consumer thread: ring'den profil al, diske yaz
        std::atomic<uint64_t> total_profiles{0};
        std::thread consumer([&ring, &total_profiles, &cfg]() {
            // run_dir varsa DiskWriter ac, yoksa nullptr (sadece say)
            std::unique_ptr<DiskWriter> writer;
            if (!cfg.run_dir.empty()) {
                writer = std::make_unique<DiskWriter>(cfg.run_dir);
                if (!writer->is_open()) writer.reset(); // Acilmazsa devre disi
            }

            Packet pkt;
            while (ring.pop(pkt)) {
                ++total_profiles;
                if (writer) {
                    writer->write(pkt.data.data(), pkt.data.size());
                }
                // Faz 4: buraya ProfileDecoder::decode(pkt) gelecek
            }
        });

        std::cout << ">>> Sistem calisiyor. Durdurmak icin Enter'a basin...\n";
        std::cin.get();

        // --- GUVENLI KAPATMA SIRASI ONEMLI ---
        std::cout << ">>> Sistem kapatiliyor...\n";
        laser.stopAcquisition(); // 1. SDK callback'i durdur
        arduino.stop();         // 2. MCU thread'i durdur
        ring.stop();            // 3. Consumer thread'in bitmesini tetikle
        consumer.join();        // 4. Consumer thread bitene kadar bekle

        std::cout << ">>> Toplam islenen profil: " << total_profiles << "\n";
        std::cout << ">>> Ring drops: " << ring.get_drops() << "\n";
        std::cout << ">>> Temiz kapandi.\n";
    }
    else {
        std::cerr << "Bilinmeyen mod: " << cfg.mode << "\n";
        std::cerr << "Gecerli modlar: mcu | laser\n";
        return 1;
    }

    return 0;
}

