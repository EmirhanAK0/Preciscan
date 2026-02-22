#pragma once
#include <thread>
#include <atomic>
#include <functional>
#include "../core/i_data_sink.h"

class McuListener {
public:
    using PacketCallback = std::function<void(uint32_t seq, float y_mm)>;

    McuListener(int port, IDataSink* sink = nullptr);
    ~McuListener();

    void start();
    void stop();

    // UI'ya veri iletmek icin: arka plan thread'inden guvenli sekilde cagrilir.
    // Qt'de queued connection ile kullanilmali (signal uzerinden).
    void setPacketCallback(PacketCallback cb) { m_packetCb = std::move(cb); }

private:
    void listenLoop();

    int m_port;
    IDataSink*     m_sink;      // Sahiplik yok
    PacketCallback m_packetCb;  // UI callback (optional)
    std::atomic<bool> m_running;
    std::thread m_worker;
};
