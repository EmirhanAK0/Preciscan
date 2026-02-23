#include "laser_manager.h"
#include <iostream>

// RT Callback — Donanim SDK'si bu fonksiyonu kendi thread'inden cagirir.
// KURAL: Burada hic bir mutex alma, heap allocate, I/O yazma yapma!
// Veri, pUserData uzerinden IDataSink'e iletilir; ne yapilacagina sink karar verir.
void _stdcall GlobalLaserCallback(const unsigned char* pData, unsigned int nSize, void* pUserData) {
    IDataSink* sink = static_cast<IDataSink*>(pUserData);
    if (sink) {
        // Lock-free: sink implementasyonu SPSCRingBuffer::try_push cagiracak
        sink->on_packet(pData, static_cast<size_t>(nSize), 0 /* ts_ns: Faz 3 */);
    } else {
        // Sink baglanmamis: minimal durum mesaji (development only)
        static unsigned int s_count = 0;
        if (++s_count % 100 == 0) { // Her 100 profilde bir log — RT kasasini gozet
            std::cout << "\r[LAZER] " << s_count << " profil alindi, sink bagli degil" << std::flush;
        }
    }
}

LaserManager::LaserManager(const std::string& dllPath, IDataSink* sink)
    : m_dllPath(dllPath), m_llt(nullptr), m_sink(sink), m_connected(false) {}

LaserManager::~LaserManager() {
    stopAcquisition();
    if (m_llt) delete m_llt;
}

bool LaserManager::init() {
    bool loadError = false;
    m_llt = new CInterfaceLLT(m_dllPath.c_str(), &loadError);
    if (loadError) {
        std::cerr << "[LAZER] HATA: " << m_dllPath << " yuklenemedi!\n";
        return false;
    }
    return true;
}

bool LaserManager::connect() {
    if (!m_llt) { m_lastError = "init() cagrilmadi"; return false; }

    // Adim 1: Mevcut cihazlari enumerate et
    const unsigned int MAX_DEVICES = 6;
    unsigned int deviceList[MAX_DEVICES] = {0};
    int nFound = m_llt->GetDeviceInterfaces(deviceList, MAX_DEVICES);

    std::cout << "[LAZER] GetDeviceInterfaces() = " << nFound << " cihaz bulundu\n";

    if (nFound < 1) {
        // Hizli enumerate de dene (UDP broadcast olmayan versiyon)
        nFound = m_llt->GetDeviceInterfacesFast(deviceList, MAX_DEVICES);
        std::cout << "[LAZER] GetDeviceInterfacesFast() = " << nFound << "\n";
    }

    if (nFound < 1) {
        m_lastError = "Agda scanCONTROL cihazi bulunamadi (lütfen Ethernet kablosunu kontrol edin)";
        return false;
    }

    // Adim 2: Bulunan ilk cihaza baglan
    for (int i = 0; i < nFound && i < (int)MAX_DEVICES; ++i) {
        m_llt->SetDeviceInterface(deviceList[i], 0);
        int ret = m_llt->Connect();
        std::cout << "[LAZER] SetDeviceInterface(" << deviceList[i] << ") -> Connect() = " << ret << "\n";
        if (ret >= GENERAL_FUNCTION_OK) {
            m_connected = true;
            m_lastError = "";
            std::cout << "[LAZER] Baglanti basarili! Cihaz interface=" << deviceList[i] << "\n";
            return true;
        }
        // Baglanti hatasi: bir sonraki
        char errStr[256] = {};
        m_llt->TranslateErrorValue(ret, errStr, sizeof(errStr));
        std::cout << "[LAZER] Hata: " << errStr << " (kod=" << ret << ")\n";
    }

    m_lastError = "Bulunan cihazlara baglanilamadi (lutfen diger uygulamalari kapatin)";
    return false;
}

void LaserManager::startAcquisition() {
    if (m_connected) {
        // m_sink, pUserData olarak callback'e geciliyor
        // Boylece GlobalLaserCallback icinde IDataSink::on_packet cagrilabilir
        m_llt->RegisterCallback(STD_CALL, (void*)GlobalLaserCallback, static_cast<void*>(m_sink));
        m_llt->TransferProfiles(NORMAL_TRANSFER, 1);
        std::cout << "[LAZER] Veri akisi baslatildi. Sink: "
                  << (m_sink ? "bagli" : "yok (log modu)") << "\n";
    }
}

void LaserManager::stopAcquisition() {
    if (m_llt && m_connected) {
        m_llt->TransferProfiles((TTransferProfileType)0, 0);
        m_llt->Disconnect();
        m_connected = false;
    }
}
