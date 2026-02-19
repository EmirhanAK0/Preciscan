#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <cstdint>
#include <atomic>

// Veri Paketi Yapısı
struct Packet {
    std::vector<uint8_t> data;
};

class PacketQueue {
public:
    explicit PacketQueue(size_t max_capacity)
        : capacity_(max_capacity), stop_signal_(false), drops_(0) {
    }

    // PRODUCER (Veri Alan) burayı çağırır
    bool try_push(Packet&& pkt) {
        std::lock_guard<std::mutex> lock(mux_);
        if (queue_.size() >= capacity_) {
            drops_++;
            return false; // Kuyruk dolu, paket düştü!
        }
        queue_.push(std::move(pkt));
        cv_.notify_one(); // Tüketiciye haber ver
        return true;
    }

    // CONSUMER (Veri İşleyen) burayı çağırır
    bool pop(Packet& out_pkt) {
        std::unique_lock<std::mutex> lock(mux_);
        // Veri gelene kadar veya dur emri gelene kadar bekle
        cv_.wait(lock, [this] { return !queue_.empty() || stop_signal_; });

        if (queue_.empty() && stop_signal_) {
            return false; // İş bitti
        }

        out_pkt = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    void stop() {
        std::lock_guard<std::mutex> lock(mux_);
        stop_signal_ = true;
        cv_.notify_all(); // Herkesi uyandır
    }

    uint64_t get_drops() const {
        std::lock_guard<std::mutex> lock(mux_);
        return drops_;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mux_);
        return queue_.size();
    }

private:
    std::queue<Packet> queue_;
    mutable std::mutex mux_; // FIX: const fonksiyonlarda kilitlenebilmesi için mutable yapıldı
    std::condition_variable cv_;
    size_t capacity_;
    bool stop_signal_;
    uint64_t drops_;
};