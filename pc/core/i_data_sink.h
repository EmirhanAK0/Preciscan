#pragma once
#include <cstdint>

// Soyut veri alici arayuzu
// McuListener, LaserManager gibi uretici siniflar bu arayuzu kullanir
// Boylece stdout, dosya, kuyruk gibi farkli hedefler kolayca takilabilir
struct IDataSink {
    virtual void on_packet(const uint8_t* data, size_t size, uint64_t ts_ns) = 0;
    virtual ~IDataSink() = default;
};
