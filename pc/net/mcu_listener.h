#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

class McuListener
{
public:
    struct TriggerEvent
    {
        uint32_t session_id;
        uint32_t seq;
        uint32_t encoder_count;
        int32_t angle_mdeg;
        uint64_t tick_us;
    };

    using PacketCallback = std::function<void(const TriggerEvent&)>;

    explicit McuListener(int port);
    ~McuListener();

    void start();
    void stop();

    bool tryGetTriggerEvent(TriggerEvent& evt);
    size_t queueSize() const;

    void setPacketCallback(PacketCallback cb)
    {
        m_packetCb = std::move(cb);
    }

private:
    void listenLoop();

    int m_port;
    PacketCallback m_packetCb;
    std::atomic<bool> m_running{false};
    std::thread m_worker;

    mutable std::mutex m_queueMutex;
    std::queue<TriggerEvent> m_queue;
};
