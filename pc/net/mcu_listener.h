#pragma once
#include <thread>
#include <atomic>
#include "../core/i_data_sink.h"

class McuListener {
public:
    // sink: veri geldiginde nereye iletilecek (nullptr = sadece say, yazma)
    McuListener(int port, IDataSink* sink = nullptr);
    ~McuListener();

    void start();
    void stop();

private:
    void listenLoop();

    int m_port;
    IDataSink* m_sink;  // Sahiplik yok, sadece pointer — lifetime disarida yonetilir
    std::atomic<bool> m_running;
    std::thread m_worker;
};
