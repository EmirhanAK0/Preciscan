#include "meta_writer.h"
#include <fstream>

static std::string esc(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c == '\\' || c == '"') out.push_back('\\');
        out.push_back(c);
    }
    return out;
}

bool write_meta_json(const std::string& path, const Config& cfg) {
    std::ofstream f(path, std::ios::out | std::ios::trunc);
    if (!f) return false;

    f << "{\n";
    f << "  \"mode\": \"" << esc(cfg.mode) << "\",\n";
    f << "  \"run_dir\": \"" << esc(cfg.run_dir) << "\",\n";
    f << "  \"port\": " << cfg.port << ",\n";
    f << "  \"udp_count\": " << cfg.udp_count << ",\n";
    f << "  \"count\": " << cfg.count << ",\n";
    f << "  \"loss\": " << cfg.loss << ",\n";
    f << "  \"ooo\": " << cfg.ooo << ",\n";
    f << "  \"dup\": " << cfg.dup << ",\n";
    f << "  \"record_out\": \"" << esc(cfg.record_out) << "\",\n";
    f << "  \"replay_in\": \"" << esc(cfg.replay_in) << "\",\n";
    f << "  \"metrics_out\": \"" << esc(cfg.metrics_out) << "\"\n";
    f << "}\n";
    return true;
}
