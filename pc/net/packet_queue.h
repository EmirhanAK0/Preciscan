#pragma once
#include "../core/packet.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

class PacketQueue {
public:
    explicit PacketQueue(size_t max_capacity)
        : capacity_(max_capacity), stop_signal_(false), drops_(0) {
    }

    bool try_push(Packet&& pkt) {
        std::lock_guard<std::mutex> lock(mux_);
        if (queue_.size() >= capacity_) {
            drops_++;
            return false;
        }
        queue_.push(std::move(pkt));
        cv_.notify_one();
        return true;
    }

    bool pop(Packet& out_pkt) {
        std::unique_lock<std::mutex> lock(mux_);
        cv_.wait(lock, [this] { return !queue_.empty() || stop_signal_; });
        if (queue_.empty() && stop_signal_) {
            return false;
        }
        out_pkt = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    void stop() {
        std::lock_guard<std::mutex> lock(mux_);
        stop_signal_ = true;
        cv_.notify_all();
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
    mutable std::mutex mux_;
    std::condition_variable cv_;
    size_t capacity_;
    bool stop_signal_;
    uint64_t drops_;
};
