#pragma once

#include <vector>
#include <atomic>
#include <cstdint>
#include <optional>
#include <thread>

// İleride daha kompleks hale gelecek paket yapısı
struct Packet {
    std::vector<uint8_t> data;
    // uint64_t timestamp_ns;
};

// SPSC (Single Producer Single Consumer) Lock-Free Ring Buffer
// Dikkat: Bu sınıf allocation (yeni bellek ayırma) yapmaz!
class SPSCRingBuffer {
public:
    explicit SPSCRingBuffer(size_t capacity) 
        : capacity_(capacity + 1), // Ring buffer mantığı gereği 1 boşluk bırakılır
          buffer_(capacity + 1), 
          head_(0), 
          tail_(0), 
          drops_(0), 
          stop_signal_(false) {}

    // --- PRODUCER (Thread A) BURAYI ÇAĞIRIR ---
    // Eğer kuyruk doluysa Drop Newest yapar ve false döner. Asla beklemez!
    bool try_push(Packet&& pkt) {
        const size_t current_head = head_.load(std::memory_order_relaxed);
        const size_t next_head = (current_head + 1) % capacity_;

        // Doluluk kontrolü (Tail'i okurken Acquire semantic)
        if (next_head == tail_.load(std::memory_order_acquire)) {
            // Buffer Dolu! (Drop Newest)
            drops_.fetch_add(1, std::memory_order_relaxed);
            return false;
        }

        // Paketi ring buffer'a yaz (Taşıma semantiği)
        buffer_[current_head] = std::move(pkt);

        // Head'i güncelle (Release semantic: Yazma işlemi bittikten sonra Head artsın)
        head_.store(next_head, std::memory_order_release);
        return true;
    }

    // --- CONSUMER (Thread B) BURAYI ÇAĞIRIR ---
    // Lock-free olduğu için condition_variable yoktur, busy-wait veya sleep yapar.
    bool pop(Packet& out_pkt) {
        while (true) {
            const size_t current_tail = tail_.load(std::memory_order_relaxed);
            
            // Eğer Head ile Tail eşitse, kuyruk boş demektir.
            if (current_tail == head_.load(std::memory_order_acquire)) {
                if (stop_signal_.load(std::memory_order_relaxed)) {
                    return false; // Çıkış emri geldi ve kuyruk bitti
                }
                
                // Active Yield (CPU'yu yormamak için kısa bir an nefes al)
                // İleride bu kısmı CPU mimarisine göre optimize edebiliriz.
                std::this_thread::yield(); 
                continue;
            }

            // Veriyi oku
            out_pkt = std::move(buffer_[current_tail]);

            // Tail'i güncelle (Release)
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
    std::vector<Packet> buffer_; // Bellek baştan ayrılır, boyut değişmez
    
    // std::atomic cache line trashing'i engellemek için align edilebilir (İleriki aşama)
    alignas(64) std::atomic<size_t> head_; // Üreticinin yazdığı yer
    alignas(64) std::atomic<size_t> tail_; // Tüketicinin okuduğu yer
    
    std::atomic<uint64_t> drops_;
    std::atomic<bool> stop_signal_;
};