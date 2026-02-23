#include "laser_manager.h"
#include <iostream>
#include <vector>

// RT Callback — SDK kendi thread'inden cagiriyor.
// KURAL: Heap alloc, mutex, I/O yapma! Sadece memcpy.
void _stdcall GlobalLaserCallback(const unsigned char* pData, unsigned int nSize, void* pUserData) {
    IDataSink* sink = static_cast<IDataSink*>(pUserData);
    if (sink && pData && nSize > 0) {
        sink->on_packet(pData, static_cast<size_t>(nSize), 0);
    }
}

LaserManager::LaserManager(const std::string& dllPath, IDataSink* sink)
    : m_dllPath(dllPath), m_sink(sink) {}

LaserManager::~LaserManager() {
    stopAcquisition();
    delete m_llt;
}

bool LaserManager::init() {
    bool loadError = false;
    m_llt = new CInterfaceLLT(m_dllPath.c_str(), &loadError);
    if (loadError) {
        m_lastError = "LLT.dll yuklenemedi: " + m_dllPath;
        std::cerr << "[LAZER] " << m_lastError << "\n";
        return false;
    }
    std::cout << "[LAZER] DLL yuklendi: " << m_dllPath << "\n";
    return true;
}

// ===========================================================================
// SDK GetProfiles_Callback orneginin tam akisi
// ===========================================================================
bool LaserManager::connect() {
    if (!m_llt) { m_lastError = "init() cagrilmadi"; return false; }

    // 1. Cihaz handle olustur
    int ret = m_llt->CreateLLTDevice(INTF_TYPE_ETHERNET);
    std::cout << "[LAZER] CreateLLTDevice = " << ret << "\n";
    if (ret < GENERAL_FUNCTION_OK) {
        m_lastError = "CreateLLTDevice basarisiz";
        return false;
    }

    // 2. Agdaki cihazlari bul
    unsigned int devList[6] = {};
    int nFound = m_llt->GetDeviceInterfaces(devList, 6);
    std::cout << "[LAZER] GetDeviceInterfaces = " << nFound << " cihaz\n";
    if (nFound < 1) {
        m_lastError = "Agda scanCONTROL bulunamadi";
        return false;
    }

    // 3. Ilk cihaza baglan
    m_llt->SetDeviceInterface(devList[0], 0);
    int connRet = m_llt->Connect();
    std::cout << "[LAZER] Connect() = " << connRet << "\n";
    if (connRet < GENERAL_FUNCTION_OK) {
        char err[256] = {};
        m_llt->TranslateErrorValue(connRet, err, sizeof(err));
        m_lastError = std::string("Connect basarisiz: ") + err;
        return false;
    }
    m_connected = true;

    // 4. Scanner tipini oku (ConvertProfile2Values icin gerekli)
    m_llt->GetLLTType(&m_scannerType);
    std::cout << "[LAZER] ScannerType = " << (int)m_scannerType << "\n";

    // 5. Cozunurlugu oku ve set et (SDK ornegi gibi ilk secenegi al)
    std::vector<DWORD> resolutions(4);
    m_llt->GetResolutions(resolutions.data(), (unsigned int)resolutions.size());
    m_resolution = resolutions[0];
    m_llt->SetResolution(m_resolution);
    std::cout << "[LAZER] Resolution = " << m_resolution << "\n";

    // 6. SDK orneginin konfigurasyonu: trigger, profil formati, exposure, idle
    m_llt->SetFeature(FEATURE_FUNCTION_TRIGGER, TRIG_INTERNAL);
    m_llt->SetProfileConfig(PROFILE);
    m_llt->SetFeature(FEATURE_FUNCTION_EXPOSURE_TIME, 100);  // 100 us
    m_llt->SetFeature(FEATURE_FUNCTION_IDLE_TIME,     500);  // 500 us (2kHz profile rate)

    // Cihaz adini logla
    char devName[128] = {}, venName[128] = {};
    m_llt->GetDeviceName(devName, sizeof(devName), venName, sizeof(venName));
    std::cout << "[LAZER] Baglandi: " << devName << " (" << venName << ")\n";
    m_lastError = "";
    return true;
}

void LaserManager::startAcquisition() {
    if (!m_connected) return;
    // SDK ornegi: once callback kaydet, sonra TransferProfiles
    m_llt->RegisterCallback(STD_CALL, (void*)GlobalLaserCallback, static_cast<void*>(m_sink));
    m_llt->TransferProfiles(NORMAL_TRANSFER, 1);
    std::cout << "[LAZER] Veri akisi AKTIF (resolution=" << m_resolution << ")\n";
}

void LaserManager::stopAcquisition() {
    if (!m_llt || !m_connected) return;
    m_llt->TransferProfiles(NORMAL_TRANSFER, 0);
    m_llt->Disconnect();
    m_connected = false;
    std::cout << "[LAZER] Veri akisi DURDURULDU\n";
}

// SDK ConvertProfile2Values: ham byte dizisi → mm cinsinden X/Z
bool LaserManager::convertProfile(const unsigned char* data, size_t /*size*/,
                                   std::vector<double>& outX, std::vector<double>& outZ) const {
    if (!m_llt || m_resolution == 0) return false;
    outX.resize(m_resolution);
    outZ.resize(m_resolution);
    int ret = m_llt->ConvertProfile2Values(
        data, m_resolution, PROFILE, m_scannerType,
        0,     // reflection 0 (ana yansima)
        true,  // mm cinsine cevirme aktif
        nullptr, nullptr, nullptr,
        outX.data(), outZ.data(),
        nullptr, nullptr);
    return (ret & CONVERT_X) && (ret & CONVERT_Z);
}
