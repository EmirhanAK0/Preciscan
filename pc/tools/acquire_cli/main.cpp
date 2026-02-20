#include <iostream>
#include <thread>
#include <chrono>
#include "mcu_listener.h"
#include "laser_manager.h"
#include "config.h"

int main(int argc, char** argv) {
    Config cfg;
    try { cfg = Config::from_cli(argc, argv); }
    catch (const std::exception& e) {
        std::cerr << "Hata: " << e.what() << "\n";
        return 1;
    }

    std::cout << "========================================\n";
    std::cout << "       PRECISCAN CORE v2.0 (MODULAR)   \n";
    std::cout << "========================================\n";
    std::cout << "Mod: " << cfg.mode << " | Port: " << cfg.port << "\n\n";

    if (cfg.mode == "mcu") {
        // Sadece MCU listener -- lazer olmadan test icin
        // IDataSink yok, McuListener kendi minimal logunu yazacak
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
        McuListener arduino(cfg.port);
        arduino.start();

        LaserManager laser("LLT.dll");
        if (laser.init()) {
            if (laser.connect()) {
                std::cout << ">>> LAZER BAGLANTISI BASARILI.\n";
                laser.startAcquisition();
            } else {
                std::cout << ">>> UYARI: Lazer bulunamadi, sadece MCU dinleniyor.\n";
            }
        }

        std::cout << ">>> Sistem calisiyor. Durdurmak icin Enter'a basin...\n";
        std::cin.get();

        std::cout << ">>> Sistem kapatiliyor...\n";
        laser.stopAcquisition();
        arduino.stop();
    }
    else {
        std::cerr << "Bu mod henuz implemente edilmedi: " << cfg.mode << "\n";
        return 1;
    }

    return 0;
}
