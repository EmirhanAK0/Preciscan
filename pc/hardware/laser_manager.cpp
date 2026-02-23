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

    // ==== SDK'NIN DOGRU AKISI (InterfaceLLT_2.cpp'den) ====
    // Adim 1: Cihaz handle'i olustur (olmadan diger tum cagrilar basarisiz olur!)
    int ret = m_llt->CreateLLTDevice(INTF_TYPE_ETHERNET);
    std::cout << "[LAZER] CreateLLTDevice(ETHERNET) = " << ret << "\n";
    if (ret < GENERAL_FUNCTION_OK) {
        m_lastError = "CreateLLTDevice basarisiz (DLL yuklendi fakat cihaz olusturulamadi)";
        return false;
    }

    // Adim 2: Agdaki scanCONTROL cihazlarini bul
    const unsigned int MAX_DEVICES = 6;
    unsigned int deviceList[MAX_DEVICES] = {0};
    int nFound = m_llt->GetDeviceInterfaces(deviceList, MAX_DEVICES);
    std::cout << "[LAZER] GetDeviceInterfaces() = " << nFound << " cihaz\n";

    if (nFound < 1) {
        m_lastError = "Agda scanCONTROL bulunamadi. Ethernet kablosunu ve kamera guc kaynagini kontrol edin.";
        return false;
    }

    // Adim 3: Ilk bulunan cihaza baglan
    m_llt->SetDeviceInterface(deviceList[0], 0);
    int connRet = m_llt->Connect();
    std::cout << "[LAZER] Connect() = " << connRet << "\n";
    if (connRet >= GENERAL_FUNCTION_OK) {
        m_connected = true;
        m_lastError = "";

        // Cihaz adini logla
        char devName[128] = {}, venName[128] = {};
        m_llt->GetDeviceName(devName, sizeof(devName), venName, sizeof(venName));
        std::cout << "[LAZER] Baglandi: " << devName << " (" << venName << ")\n";
        return true;
    }

    // Hata kodunu insan-okunabilir stringe cevir
    char errStr[256] = {};
    m_llt->TranslateErrorValue(connRet, errStr, sizeof(errStr));
    m_lastError = std::string("Connect basarisiz: ") + errStr + " (kod=" + std::to_string(connRet) + ")";
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
