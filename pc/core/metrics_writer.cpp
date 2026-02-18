#include "metrics_writer.h"
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

bool write_metrics_json(
    const std::string& path,
    const std::string& mode,
    int port,
    uint32_t target_count,
    const SeqStats& st,
    double plr,
    double time_s
) {
    std::ofstream f(path, std::ios::out | std::ios::trunc);
    if (!f) return false;

    f << "{\n";
    f << "  \"mode\": \"" << esc(mode) << "\",\n";
    f << "  \"port\": " << port << ",\n";
    f << "  \"target_count\": " << target_count << ",\n";
    f << "  \"rx\": " << st.rx << ",\n";
    f << "  \"missing\": " << st.missing << ",\n";
    f << "  \"ooo\": " << st.ooo << ",\n";
    f << "  \"dup\": " << st.dup << ",\n";
    f << "  \"plr\": " << plr << ",\n";
    f << "  \"time_s\": " << time_s << "\n";
    f << "}\n";
    return true;
}
