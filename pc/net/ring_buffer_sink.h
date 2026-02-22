#pragma once
#include "../core/i_data_sink.h"
#include "spsc_ring_buffer.h"

// IDataSink implementasyonu: gelen veriyi SPSCRingBuffer'a kopyalar.
// Callback thread (producer) bu sinifi cagirir.
// Consumer thread ayri bir thread'te ring'den okur.
//
// ONEMLI: on_packet() lock-free ve non-blocking'dir.
// Kuyruk doluysa paket duser (drop newest) ve false doner.
class RingBufferSink : public IDataSink {
public:
    explicit RingBufferSink(SPSCRingBuffer& ring) : m_ring(ring) {}

    void on_packet(const uint8_t* data, size_t size, uint64_t /*ts_ns*/) override {
        Packet pkt;
        pkt.data.assign(data, data + size); // Ham byte kopyasi
        m_ring.try_push(std::move(pkt));    // Kuyruk dolu ise drop, asla bloklanmaz
    }

private:
    SPSCRingBuffer& m_ring; // Sahiplik yok — lifetime disarida yonetilir
};
