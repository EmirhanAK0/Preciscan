#include "disk_writer.h"
#include <iostream>

DiskWriter::DiskWriter(const std::string& run_dir) {
    const std::string path = run_dir + "/frames.bin";
    m_file.open(path, std::ios::binary | std::ios::trunc);
    if (!m_file) {
        std::cerr << "[DISK] HATA: Dosya acilamadi: " << path << "\n";
    } else {
        std::cout << "[DISK] Kayit basladi: " << path << "\n";
    }
}

DiskWriter::~DiskWriter() {
    if (m_file.is_open()) {
        m_file.flush();
        m_file.close();
        std::cout << "[DISK] Kapandi. Yazilan: "
                  << m_frames << " frame, "
                  << m_bytes  << " byte\n";
    }
}

bool DiskWriter::write(const uint8_t* data, size_t size) {
    if (!m_file) return false;

    // Frame format: [uint32_t boyut (LE)][ham veri]
    const uint32_t sz = static_cast<uint32_t>(size);
    m_file.write(reinterpret_cast<const char*>(&sz), sizeof(sz));
    m_file.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));

    if (!m_file) {
        std::cerr << "[DISK] HATA: Yazma basarisiz!\n";
        return false;
    }

    ++m_frames;
    m_bytes += sizeof(sz) + size;
    return true;
}
