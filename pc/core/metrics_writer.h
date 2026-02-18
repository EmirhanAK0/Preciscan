#pragma once
#include <string>
#include "seq_metrics.h"

bool write_metrics_json(
    const std::string& path,
    const std::string& mode,
    int port,
    uint32_t target_count,
    const SeqStats& st,
    double plr,
    double time_s
);
