#include "seq_metrics.h"

// uint32_t wrap-around aritmetigi hakkinda not:
// uint32_t cikarma islemi modular aritmetik kullanir (mod 2^32)
// Yani: (uint32_t)(0x00000000 - 0xFFFFFFFF) = 1 -- dogru sonuc
// std::max kullanilamaz cunku wrap-around sonrasi kucuk gorunen seq aslinda buyuktur

void SeqTracker::update(uint32_t seq) {
    if (!has_last) {
        has_last = true;
        last = seq;
        stats.rx += 1;
        return;
    }

    // Fark her zaman uint32_t aritmetigi ile hesaplanir
    // Bu sayede wrap-around (0xFFFFFFFF -> 0x00000000) seffaf sekilde calisir
    const uint32_t diff = seq - last;

    if (diff == 0) {
        // Ayni sequence tekrar geldi
        stats.dup += 1;
    } else if (diff == 1) {
        // Beklenen siradaki paket -- ideal durum, sayac yok
    } else if (diff < 0x80000000u) {
        // diff kucukse ileri gitmis demektir: aradaki paketler kayip
        // Ornek: last=5, seq=8 -> diff=3 -> 2 paket kayip
        // Wrap-around: last=0xFFFFFFFE, seq=2 -> diff=4 -> 3 paket kayip
        stats.missing += static_cast<uint64_t>(diff - 1);
    } else {
        // diff >= 0x80000000: seq geriye gitmis demektir -> sira disi
        // Ornek: last=10, seq=5 -> diff=0xFFFFFFFB -> ooo
        stats.ooo += 1;
    }

    stats.rx += 1;

    // last'i guncelle: sadece seq gercekten ilerlemisse (diff < 0x80000000)
    if (diff != 0 && diff < 0x80000000u) {
        last = seq;
    }
}

double SeqTracker::plr() const {
    const double denom = static_cast<double>(stats.rx + stats.missing);
    return denom > 0.0 ? static_cast<double>(stats.missing) / denom : 0.0;
}
