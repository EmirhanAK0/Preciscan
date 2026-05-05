#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

class SerialTriggerReader
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

    SerialTriggerReader(const std::string& port_name, uint32_t baudrate = 115200);
    ~SerialTriggerReader();

    bool start();
    void stop();

    bool tryGetTriggerEvent(TriggerEvent& evt);
    size_t queueSize() const;

    /// Arduino'ya komut gonder (ornekin "ZMOVE:15.0\n")
    bool sendCommand(const std::string& cmd);

private:
    void readLoop();
    bool openPort();
    void closePort();
    bool configurePort();
    bool parseLine(const std::string& line, TriggerEvent& evt);

private:
    std::string m_portName;
    uint32_t m_baudrate;

    void* m_handle = nullptr;
    std::atomic<bool> m_running{false};
    std::thread m_worker;

    mutable std::mutex m_queueMutex;
    std::queue<TriggerEvent> m_queue;

    std::mutex m_writeMutex;
};