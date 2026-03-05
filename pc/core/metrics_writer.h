#pragma once
#include "seq_metrics.h"

#include <string>

bool write_metrics_json(const std::string& path,
                        const std::string& mode,
                        int port,
                        uint32_t target_count,
                        const SeqStats& st,
                        double plr,
                        double time_s);
