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
    if (!m_llt) return false;
    m_llt->SetDeviceInterface(INTF_TYPE_ETHERNET, 0);
    if (m_llt->Connect() >= 1) {
        m_connected = true;
        return true;
    }
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
