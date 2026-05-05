#include "serial_trigger_reader.h"

#include <windows.h>
#include <iostream>
#include <sstream>
#include <vector>

namespace
{
    std::vector<std::string> split(const std::string& s, char delim)
    {
        std::vector<std::string> parts;
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim))
            parts.push_back(item);
        return parts;
    }
}

SerialTriggerReader::SerialTriggerReader(const std::string& port_name, uint32_t baudrate)
    : m_portName(port_name), m_baudrate(baudrate)
{
}

SerialTriggerReader::~SerialTriggerReader()
{
    stop();
}

bool SerialTriggerReader::sendCommand(const std::string& cmd)
{
    std::lock_guard<std::mutex> lock(m_writeMutex);

    if (!m_handle)
    {
        std::cerr << "[SERIAL] sendCommand: port acik degil\n";
        return false;
    }

    // Satir sonu yoksa ekle
    std::string data = cmd;
    if (data.empty() || data.back() != '\n')
        data.push_back('\n');

    HANDLE h = static_cast<HANDLE>(m_handle);
    DWORD written = 0;
    BOOL ok = WriteFile(h, data.c_str(), (DWORD)data.size(), &written, nullptr);

    if (!ok || written != (DWORD)data.size())
    {
        std::cerr << "[SERIAL] sendCommand yazma hatasi: " << GetLastError() << "\n";
        return false;
    }

    std::cout << "[SERIAL] Gonderildi: " << cmd << "\n";
    return true;
}

bool SerialTriggerReader::start()
{
    if (m_running)
        return true;

    if (!openPort())
        return false;

    if (!configurePort())
    {
        closePort();
        return false;
    }

    m_running = true;
    m_worker = std::thread(&SerialTriggerReader::readLoop, this);
    return true;
}

void SerialTriggerReader::stop()
{
    m_running = false;

    if (m_worker.joinable())
        m_worker.join();

    closePort();
}

bool SerialTriggerReader::tryGetTriggerEvent(TriggerEvent& evt)
{
    std::lock_guard<std::mutex> lock(m_queueMutex);
    if (m_queue.empty())
        return false;

    evt = m_queue.front();
    m_queue.pop();
    return true;
}

size_t SerialTriggerReader::queueSize() const
{
    std::lock_guard<std::mutex> lock(m_queueMutex);
    return m_queue.size();
}

bool SerialTriggerReader::openPort()
{
    std::string winPort = "\\\\.\\" + m_portName;

    HANDLE h = CreateFileA(
        winPort.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr);

    if (h == INVALID_HANDLE_VALUE)
    {
        std::cerr << "[SERIAL] Port acilamadi: " << m_portName
                  << " hata=" << GetLastError() << "\n";
        return false;
    }

    m_handle = h;
    return true;
}

void SerialTriggerReader::closePort()
{
    if (m_handle)
    {
        CloseHandle(static_cast<HANDLE>(m_handle));
        m_handle = nullptr;
    }
}

bool SerialTriggerReader::configurePort()
{
    HANDLE h = static_cast<HANDLE>(m_handle);

    DCB dcb{};
    dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(h, &dcb))
    {
        std::cerr << "[SERIAL] GetCommState hata=" << GetLastError() << "\n";
        return false;
    }

    dcb.BaudRate = m_baudrate;
    dcb.ByteSize = 8;
    dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fBinary  = TRUE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;

    if (!SetCommState(h, &dcb))
    {
        std::cerr << "[SERIAL] SetCommState hata=" << GetLastError() << "\n";
        return false;
    }

    COMMTIMEOUTS timeouts{};
    timeouts.ReadIntervalTimeout         = 20;
    timeouts.ReadTotalTimeoutConstant    = 20;
    timeouts.ReadTotalTimeoutMultiplier  = 1;
    timeouts.WriteTotalTimeoutConstant   = 20;
    timeouts.WriteTotalTimeoutMultiplier = 1;

    if (!SetCommTimeouts(h, &timeouts))
    {
        std::cerr << "[SERIAL] SetCommTimeouts hata=" << GetLastError() << "\n";
        return false;
    }

    PurgeComm(h, PURGE_RXCLEAR | PURGE_TXCLEAR);
    return true;
}

bool SerialTriggerReader::parseLine(const std::string& line, TriggerEvent& evt)
{
    // Eger satir virgullerle ayrilmissa eski formati destekle
    auto parts = split(line, ',');
    if (parts.size() == 6 && parts[0] == "T")
    {
        try
        {
            evt.session_id    = static_cast<uint32_t>(std::stoul(parts[1]));
            evt.seq           = static_cast<uint32_t>(std::stoul(parts[2]));
            evt.encoder_count = static_cast<uint32_t>(std::stoul(parts[3]));
            evt.angle_mdeg    = static_cast<int32_t>(std::stol(parts[4]));
            evt.tick_us       = static_cast<uint64_t>(std::stoull(parts[5]));
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    // Aksi halde kullanicinin Arduino kodundaki gibi sadece float (ornegin 45.23) bekliyoruz
    try
    {
        // Gelen yalnizca float bir string ise parse et (ornek: "45.23")
        float angleDeg = std::stof(line);
        evt.session_id    = 1;
        evt.seq           = 0; // seq su an kullanilmiyor ama arduino eklemiyor
        evt.encoder_count = 0;
        evt.angle_mdeg    = static_cast<int32_t>(angleDeg * 1000.0f);
        evt.tick_us       = 0;
        return true;
    }
    catch (...)
    {
        // Float degilse (ornek: "Sistem basladi", "Limit switch basildi" gibi text)
        return false;
    }
}

void SerialTriggerReader::readLoop()
{
    HANDLE h = static_cast<HANDLE>(m_handle);

    std::string lineBuffer;
    char ch = 0;
    DWORD bytesRead = 0;

    std::cout << "[SERIAL] Dinleme basladi: " << m_portName << "\n";

    while (m_running)
    {
        if (!ReadFile(h, &ch, 1, &bytesRead, nullptr))
        {
            std::cerr << "[SERIAL] ReadFile hata=" << GetLastError() << "\n";
            Sleep(10);
            continue;
        }

        if (bytesRead == 0)
        {
            Sleep(1);
            continue;
        }

        if (ch == '\r')
            continue;

        if (ch == '\n')
        {
            if (!lineBuffer.empty())
            {
                TriggerEvent evt;
                if (parseLine(lineBuffer, evt))
                {
                    std::lock_guard<std::mutex> lock(m_queueMutex);
                    m_queue.push(evt);
                }
                lineBuffer.clear();
            }
        }
        else
        {
            lineBuffer.push_back(ch);
        }
    }
}