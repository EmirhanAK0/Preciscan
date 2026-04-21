#include "config.h"
#include "laser_manager.h"
#include "mcu_listener.h"
#include "ring_buffer_sink.h"
#include "spsc_ring_buffer.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <memory>
#include <vector>

#include "../../io/disk_writer.h"

struct MatchedFrame
{
    McuListener::TriggerEvent trigger;
    Packet profile;
};

int main(int argc, char** argv)
{
    Config cfg;
    try
    {
        cfg = Config::from_cli(argc, argv);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Hata: " << e.what() << "\n";
        return 1;
    }

    std::cout << "========================================\n";
    std::cout << "   PRECISCAN CORE v2.2 (DUAL-STREAM)   \n";
    std::cout << "========================================\n";
    std::cout << "Mod: " << cfg.mode << " | Port: " << cfg.port << "\n\n";

    if (cfg.mode == "mcu")
    {
        McuListener mcu(cfg.port);
        mcu.start();

        std::atomic<uint64_t> total_triggers{0};
        std::atomic<bool> monitor_running{true};

        std::thread monitor([&]()
        {
            McuListener::TriggerEvent evt;
            while (monitor_running)
            {
                while (mcu.tryGetTriggerEvent(evt))
                {
                    ++total_triggers;
                    std::cout << "\r[MCU] seq=" << evt.seq
                              << " enc=" << evt.encoder_count
                              << " angle=" << evt.angle_mdeg
                              << " tick=" << evt.tick_us
                              << "        " << std::flush;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        });

        std::cout << "\n[ANA] MCU dinleniyor (UDP port " << cfg.port << ").\n";
        std::cout << "[ANA] Durdurmak icin Enter'a basin...\n";
        std::cin.get();

        std::cout << "\n[ANA] Durduruluyor...\n";
        monitor_running = false;
        mcu.stop();
        if (monitor.joinable())
            monitor.join();

        std::cout << "[ANA] Toplam trigger: " << total_triggers << "\n";
        std::cout << "[ANA] Temiz kapandi.\n";
    }
    else if (cfg.mode == "laser")
    {
        SPSCRingBuffer ring(256);
        RingBufferSink sink(ring);

        McuListener mcu(cfg.port);
        mcu.start();

        LaserManager laser("LLT.dll", &sink);
        bool laser_connected = false;

        if (laser.init())
        {
            if (laser.connect())
            {
                std::cout << ">>> LAZER BAGLANTISI BASARILI.\n";
                laser.startAcquisition();
                laser_connected = true;
            }
            else
            {
                std::cout << ">>> UYARI: Lazer bulunamadi, sadece MCU dinleniyor.\n";
            }
        }
        else
        {
            std::cout << ">>> UYARI: Laser init basarisiz.\n";
        }

        std::atomic<uint64_t> total_profiles{0};
        std::atomic<uint64_t> total_triggers{0};
        std::atomic<uint64_t> total_matched{0};

        std::mutex matchedMutex;
        std::vector<MatchedFrame> matchedFrames;

        std::thread consumer([&]()
        {
            std::unique_ptr<DiskWriter> writer;
            if (!cfg.run_dir.empty())
            {
                writer = std::make_unique<DiskWriter>(cfg.run_dir);
                if (!writer->is_open())
                    writer.reset();
            }

            std::queue<Packet> profileQueue;
            std::queue<McuListener::TriggerEvent> triggerQueue;

            Packet pkt;
            McuListener::TriggerEvent evt;

            while (true)
            {
                while (mcu.tryGetTriggerEvent(evt))
                {
                    triggerQueue.push(evt);
                    ++total_triggers;
                }

                if (!ring.pop(pkt))
                {
                    break;
                }

                profileQueue.push(pkt);
                ++total_profiles;

                while (!triggerQueue.empty() && !profileQueue.empty())
                {
                    MatchedFrame mf;
                    mf.trigger = triggerQueue.front();
                    mf.profile = profileQueue.front();

                    triggerQueue.pop();
                    profileQueue.pop();

                    ++total_matched;

                    if (writer)
                    {
                        writer->write(mf.profile.data.data(), mf.profile.data.size());
                    }

                    {
                        std::lock_guard<std::mutex> lock(matchedMutex);
                        matchedFrames.push_back(std::move(mf));
                    }
                }
            }

            while (mcu.tryGetTriggerEvent(evt))
            {
                triggerQueue.push(evt);
                ++total_triggers;
            }

            std::cout << "\n>>> Kalan trigger queue: " << triggerQueue.size() << "\n";
            std::cout << ">>> Kalan profile queue: " << profileQueue.size() << "\n";
        });

        std::cout << ">>> Sistem calisiyor. Durdurmak icin Enter'a basin...\n";
        std::cin.get();

        std::cout << ">>> Sistem kapatiliyor...\n";

        if (laser_connected)
        {
            laser.stopAcquisition();
        }

        mcu.stop();
        ring.stop();

        if (consumer.joinable())
            consumer.join();

        std::cout << ">>> Toplam trigger: " << total_triggers << "\n";
        std::cout << ">>> Toplam profil: " << total_profiles << "\n";
        std::cout << ">>> Toplam eslesen frame: " << total_matched << "\n";
        std::cout << ">>> Ring drops: " << ring.get_drops() << "\n";

        {
            std::lock_guard<std::mutex> lock(matchedMutex);
            if (!matchedFrames.empty())
            {
                const auto& first = matchedFrames.front();
                const auto& last  = matchedFrames.back();

                std::cout << ">>> Ilk eslesme: seq=" << first.trigger.seq
                          << " angle=" << first.trigger.angle_mdeg << "\n";
                std::cout << ">>> Son eslesme: seq=" << last.trigger.seq
                          << " angle=" << last.trigger.angle_mdeg << "\n";
            }
        }

        std::cout << ">>> Temiz kapandi.\n";
    }
    else
    {
        std::cerr << "Bilinmeyen mod: " << cfg.mode << "\n";
        std::cerr << "Gecerli modlar: mcu | laser\n";
        return 1;
    }

    return 0;
}
