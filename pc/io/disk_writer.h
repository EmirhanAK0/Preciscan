#pragma once
#include <string>
#include <fstream>
#include <cstdint>

// Ham profil verilerini binary dosyaya yazan sinif.
// Her frame su formatta yazilir:
//   [uint32_t size_bytes] [size_bytes adet raw byte]
// Bu format ProfileDecoder tarafindan okunabilir (Faz 4).
//
// Kullanim:
//   DiskWriter w("runs/run_001");
//   if (w.is_open()) w.write(pkt);
//   // Destructor otomatik kapatir (RAII)
class DiskWriter {
public:
    // run_dir: "runs/run_001" gibi bir klasor yolu.
    // Klasor onceden olusturulmalidir (main.cpp sorumludur).
    // frames.bin dosyasini bu klasorde acar.
    explicit DiskWriter(const std::string& run_dir);
    ~DiskWriter(); // flush + kapat

    bool is_open() const { return m_file.is_open(); }

    // Tek bir profil yaz. Basarisizsa false doner.
    bool write(const uint8_t* data, size_t size);

    uint64_t frames_written() const { return m_frames; }
    uint64_t bytes_written()  const { return m_bytes;  }

    // Kopyalamaya izin yok (dosya handle RAII)
    DiskWriter(const DiskWriter&) = delete;
    DiskWriter& operator=(const DiskWriter&) = delete;

private:
    std::ofstream m_file;
    uint64_t      m_frames = 0;
    uint64_t      m_bytes  = 0;
};
