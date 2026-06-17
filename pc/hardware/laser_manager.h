#pragma once

#include <string>
#include <vector>

#include "../core/i_data_sink.h"
#include "InterfaceLLT_2.h"

class LaserManager
{
public:
    enum class TriggerMode
    {
        Internal,
        ExternalDigitalIn
    };

public:
    LaserManager(const std::string& dllPath = "LLT.dll", IDataSink* sink = nullptr);
    ~LaserManager();

    bool init();
    bool connect();
    void startAcquisition();
    void stopAcquisition();
    void disconnect();

    bool applyAcquisitionSettings();

    bool convertProfile(const unsigned char* data,
                        size_t size,
                        std::vector<double>& outX,
                        std::vector<double>& outZ) const;

    unsigned int resolution() const
    {
        return m_resolution;
    }

    TScannerType scannerType() const
    {
        return m_scannerType;
    }

    std::string getLastError() const
    {
        return m_lastError;
    }

    /// En son kesfedilen cihaz IP'si (a.b.c.d). Tani/ag ayari icin.
    std::string deviceIp() const
    {
        return m_deviceIp;
    }

    bool isConnected() const
    {
        return m_connected;
    }

    void setProfileRateHz(int hz);
    void setExposureTimeUs(int us);
    void setAutoExposure(bool enabled);
    void setMeasuringField(const std::string& field);
    void setPointsPerProfile(unsigned int points);
    void setTriggerMode(TriggerMode mode);

    int profileRateHz() const
    {
        return m_profileRateHz;
    }

    int exposureTimeUs() const
    {
        return m_exposureTimeUs;
    }

    bool autoExposure() const
    {
        return m_autoExposure;
    }

    std::string measuringField() const
    {
        return m_measuringField;
    }

    unsigned int pointsPerProfile() const
    {
        return m_pointsPerProfile;
    }

    TriggerMode triggerMode() const
    {
        return m_triggerMode;
    }

private:
    bool validateTiming(std::string* err = nullptr) const;
    unsigned int chooseResolutionFromPoints(unsigned int points) const;
    DWORD buildExposureValue() const;
    DWORD buildMeasuringFieldValue() const;
    DWORD buildTriggerValue() const;

private:
    std::string m_dllPath;
    std::string m_lastError;
    std::string m_deviceIp;

    CInterfaceLLT* m_llt = nullptr;
    IDataSink* m_sink    = nullptr;

    bool m_connected = false;
    bool m_acquiring = false;
    bool m_deviceCreated = false;

    unsigned int m_resolution  = 0;
    TScannerType m_scannerType = StandardType;

    int m_profileRateHz             = 100;
    int m_exposureTimeUs            = 100;
    bool m_autoExposure             = true;
    std::string m_measuringField    = "large";
    unsigned int m_pointsPerProfile = 1280;
    TriggerMode m_triggerMode       = TriggerMode::Internal;
};
