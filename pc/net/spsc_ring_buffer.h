#pragma once
#include "../core/packet.h"
#include <atomic>
#include <vector>
#include <thread>

class SPSCRingBuffer {
public:
    explicit SPSCRingBuffer(size_t capacity)
        : capacity_(capacity + 1),
          buffer_(capacity + 1),
          head_(0),
          tail_(0),
          drops_(0),
          stop_signal_(false) {}

    bool try_push(Packet&& pkt) {
        const size_t current_head = head_.load(std::memory_order_relaxed);
        const size_t next_head = (current_head + 1) % capacity_;
        if (next_head == tail_.load(std::memory_order_acquire)) {
            drops_.fetch_add(1, std::memory_order_relaxed);
            return false;
        }
        buffer_[current_head] = std::move(pkt);
        head_.store(next_head, std::memory_order_release);
        return true;
    }

    bool pop(Packet& out_pkt) {
        while (true) {
            const size_t current_tail = tail_.load(std::memory_order_relaxed);
            if (current_tail == head_.load(std::memory_order_acquire)) {
                if (stop_signal_.load(std::memory_order_relaxed)) {
                    return false;
                }
                std::this_thread::yield();
                continue;
            }
            out_pkt = std::move(buffer_[current_tail]);
            tail_.store((current_tail + 1) % capacity_, std::memory_order_release);
            return true;
        }
    }

    void stop() {
        stop_signal_.store(true, std::memory_order_relaxed);
    }

    uint64_t get_drops() const {
        return drops_.load(std::memory_order_relaxed);
    }

private:
    size_t capacity_;
    std::vector<Packet> buffer_;
    alignas(64) std::atomic<size_t> head_;
    alignas(64) std::atomic<size_t> tail_;
    std::atomic<uint64_t> drops_;
    std::atomic<bool> stop_signal_;
};
