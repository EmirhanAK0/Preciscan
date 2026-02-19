#include "seq_metrics.h"
#include <algorithm>

void SeqTracker::update(uint32_t seq) {
    if (!has_last) {
        has_last = true;
        last = seq;
        stats.rx += 1;
        return;
    }

    if (seq == last) {
        stats.dup += 1;
    } else if (seq == last + 1) {
        // ok
    } else if (seq > last + 1) {
        stats.missing += static_cast<uint64_t>(seq - last - 1);
    } else {
        // seq < last
        stats.ooo += 1;
    }

    stats.rx += 1;
    last = std::max(last, seq);
}

double SeqTracker::plr() const {
    const double denom = static_cast<double>(stats.rx + stats.missing);
    return denom > 0.0 ? static_cast<double>(stats.missing) / denom : 0.0;
}
