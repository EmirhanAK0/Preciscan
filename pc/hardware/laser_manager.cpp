#include "laser_manager.h"

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

// RT Callback — SDK kendi thread'inden cagiriyor.
// KURAL: Heap alloc, mutex, I/O yapma! Sadece memcpy.
void _stdcall GlobalLaserCallback(const unsigned char* pData, unsigned int nSize, void* pUserData)
{
    IDataSink* sink = static_cast<IDataSink*>(pUserData);
    if (sink && pData && nSize > 0)
    {
        sink->on_packet(pData, static_cast<size_t>(nSize), 0);
    }
}

LaserManager::LaserManager(const std::string& dllPath, IDataSink* sink)
    : m_dllPath(dllPath), m_sink(sink)
{
}

LaserManager::~LaserManager()
{
    stopAcquisition();
    disconnect();
    delete m_llt;
    m_llt = nullptr;
}

bool LaserManager::init()
{
    if (m_llt)
        return true;

    bool loadError = false;
    m_llt = new CInterfaceLLT(m_dllPath.c_str(), &loadError);

    if (loadError)
    {
        m_lastError = "LLT.dll yuklenemedi: " + m_dllPath;
        std::cerr << "[LAZER] " << m_lastError << "\n";
        delete m_llt;
        m_llt = nullptr;
        return false;
    }

    std::cout << "[LAZER] DLL yuklendi: " << m_dllPath << "\n";
    return true;
}

bool LaserManager::validateTiming(std::string* err) const
{
    if (m_profileRateHz <= 0)
    {
        if (err)
            *err = "Profil hizi sifirdan buyuk olmalidir.";
        return false;
    }

    const int periodUs = 1000000 / m_profileRateHz;
    if (m_exposureTimeUs >= periodUs)
    {
        if (err)
            *err = "Pozlama suresi, secilen profil hizinin periyodundan kucuk olmalidir.";
        return false;
    }

    return true;
}

unsigned int LaserManager::chooseResolutionFromPoints(unsigned int points) const
{
    switch (points)
    {
    case 160:
    case 320:
    case 640:
    case 1280:
        return points;
    default:
        return 1280;
    }
}

DWORD LaserManager::buildExposureValue() const
{
    DWORD value = static_cast<DWORD>(m_exposureTimeUs);
    if (m_autoExposure)
        value |= EXPOSURE_AUTOMATIC;
    return value;
}

DWORD LaserManager::buildMeasuringFieldValue() const
{
    // Quick reference'ta predefined fields:
    // large -> 0
    // standard -> 2
    // small -> 7
    // Bu indeksler FEATURE_FUNCTION_ROI1_PRESET / deprecated MEASURINGFIELD için kullanılır.
    if (m_measuringField == "large")
        return 0;
    if (m_measuringField == "standard")
        return 2;
    if (m_measuringField == "small")
        return 7;

    return 0;
}

bool LaserManager::applyAcquisitionSettings()
{
    if (!m_llt || !m_connected)
    {
        m_lastError = "Cihaz bagli degil";
        return false;
    }

    std::string timingErr;
    if (!validateTiming(&timingErr))
    {
        m_lastError = timingErr;
        std::cerr << "[LAZER] " << m_lastError << "\n";
        return false;
    }

    // 1) Resolution / points per profile
    m_resolution = chooseResolutionFromPoints(m_pointsPerProfile);
    int retRes = m_llt->SetResolution(m_resolution);
    std::cout << "[LAZER] SetResolution(" << m_resolution << ") = " << retRes << "\n";

    // 2) Measuring field preset
    const DWORD measuringFieldValue = buildMeasuringFieldValue();
    int retField = m_llt->SetFeature(FEATURE_FUNCTION_ROI1_PRESET, measuringFieldValue);
    std::cout << "[LAZER] SetFeature(ROI1_PRESET=" << measuringFieldValue << ") = " << retField << "\n";

    // 3) Trigger + profile config
    DWORD trig = buildTriggerValue();
    int retTrig = m_llt->SetFeature(FEATURE_FUNCTION_TRIGGER, trig);
    std::cout << "[LAZER] SetFeature(TRIGGER="
              << (m_triggerMode == TriggerMode::Internal ? "INTERNAL" : "EXTERNAL")
              << ") = " << retTrig << "\n";

    int retCfg = m_llt->SetProfileConfig(PROFILE);
    std::cout << "[LAZER] SetProfileConfig(PROFILE) = " << retCfg << "\n";

    // 4) Exposure
    const DWORD exposureValue = buildExposureValue();
    int retExp = m_llt->SetFeature(FEATURE_FUNCTION_EXPOSURE_TIME, exposureValue);
    std::cout << "[LAZER] SetFeature(EXPOSURE=" << exposureValue << ") = " << retExp << "\n";

    // 5) Idle = period - exposure
    const int periodUs = 1000000 / m_profileRateHz;
    int idleUs = periodUs - m_exposureTimeUs;
    if (idleUs < 1)
        idleUs = 1;

    int retIdle = m_llt->SetFeature(FEATURE_FUNCTION_IDLE_TIME, static_cast<DWORD>(idleUs));
    std::cout << "[LAZER] SetFeature(IDLE=" << idleUs << " us) = " << retIdle << "\n";

    m_lastError.clear();
    return true;
}

DWORD LaserManager::buildTriggerValue() const
{
    if (m_triggerMode == TriggerMode::Internal)
    {
        return TRIG_INTERNAL;
    }
    else
    {
        return TRIG_EXT_ACTIVE
             | TRIG_INPUT_DIGIN
             | TRIG_MODE_EDGE
             | TRIG_POLARITY_HIGH;
    }
}

bool LaserManager::connect()
{
    if (!m_llt)
    {
        m_lastError = "init() cagrilmadi";
        return false;
    }

    if (m_connected)
        return true;

    if (!m_deviceCreated)
    {
        int ret = m_llt->CreateLLTDevice(INTF_TYPE_ETHERNET);
        std::cout << "[LAZER] CreateLLTDevice = " << ret << "\n";
        if (ret < GENERAL_FUNCTION_OK)
        {
            m_lastError = "CreateLLTDevice basarisiz";
            return false;
        }
        m_deviceCreated = true;
    }

    unsigned int devList[6] = {};
    int nFound = 0;
    
    // Cihaz agda hemen bulunamayabilir (broadcast yanit suresi), bu yuzden Configuration Tools 
    // gibi birkac kez tekrar denememiz gerekiyor.
    for (int retry = 0; retry < 5; ++retry)
    {
        nFound = m_llt->GetDeviceInterfaces(devList, 6);
        if (nFound >= 1)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
    
    std::cout << "[LAZER] GetDeviceInterfaces = " << nFound << " cihaz\n";
    if (nFound < 1)
    {
        m_lastError = "Agda scanCONTROL bulunamadi";
        return false;
    }

    m_llt->SetDeviceInterface(devList[0], 0);

    int connRet = m_llt->Connect();
    std::cout << "[LAZER] Connect() = " << connRet << "\n";
    if (connRet < GENERAL_FUNCTION_OK)
    {
        char err[256] = {};
        m_llt->TranslateErrorValue(connRet, err, sizeof(err));
        m_lastError = std::string("Connect basarisiz: ") + err;
        return false;
    }

    m_connected = true;

    m_llt->GetLLTType(&m_scannerType);
    std::cout << "[LAZER] ScannerType = " << static_cast<int>(m_scannerType) << "\n";

    if (!applyAcquisitionSettings())
    {
        disconnect();
        return false;
    }

    char devName[128] = {}, venName[128] = {};
    m_llt->GetDeviceName(devName, sizeof(devName), venName, sizeof(venName));
    std::cout << "[LAZER] Baglandi: " << devName << " (" << venName << ")\n";

    return true;
}

void LaserManager::startAcquisition()
{
    if (!m_connected || !m_llt || m_acquiring)
        return;

    m_llt->RegisterCallback(STD_CALL, (void*)GlobalLaserCallback, static_cast<void*>(m_sink));
    m_llt->TransferProfiles(NORMAL_TRANSFER, 1);
    m_acquiring = true;

    std::cout << "[LAZER] Veri akisi AKTIF"
              << " (resolution=" << m_resolution
              << ", rate=" << m_profileRateHz
              << " Hz, exposure=" << m_exposureTimeUs
              << " us, auto=" << (m_autoExposure ? "on" : "off")
              << ", field=" << m_measuringField
              << ")\n";
}

void LaserManager::stopAcquisition()
{
    if (!m_llt || !m_connected || !m_acquiring)
        return;

    m_llt->TransferProfiles(NORMAL_TRANSFER, 0);
    m_acquiring = false;

    std::cout << "[LAZER] Veri akisi DURDURULDU\n";
}

void LaserManager::disconnect()
{
    if (!m_llt || !m_connected)
        return;

    if (m_acquiring)
        stopAcquisition();

    m_llt->Disconnect();
    m_connected = false;

    std::cout << "[LAZER] Cihaz baglantisi kapatildi\n";
}

bool LaserManager::convertProfile(const unsigned char* data,
                                  size_t /*size*/,
                                  std::vector<double>& outX,
                                  std::vector<double>& outZ) const
{
    if (!m_llt || m_resolution == 0)
        return false;

    outX.resize(m_resolution);
    outZ.resize(m_resolution);

    int ret = m_llt->ConvertProfile2Values(data,
                                           m_resolution,
                                           PROFILE,
                                           m_scannerType,
                                           0,
                                           true,
                                           nullptr,
                                           nullptr,
                                           nullptr,
                                           outX.data(),
                                           outZ.data(),
                                           nullptr,
                                           nullptr);

    return (ret & CONVERT_X) && (ret & CONVERT_Z);
}

void LaserManager::setTriggerMode(TriggerMode mode)
{
    m_triggerMode = mode;
}

void LaserManager::setProfileRateHz(int hz)
{
    m_profileRateHz = hz;
}

void LaserManager::setExposureTimeUs(int us)
{
    m_exposureTimeUs = us;
}

void LaserManager::setAutoExposure(bool enabled)
{
    m_autoExposure = enabled;
}

void LaserManager::setMeasuringField(const std::string& field)
{
    m_measuringField = field;
}

void LaserManager::setPointsPerProfile(unsigned int points)
{
    m_pointsPerProfile = points;
}