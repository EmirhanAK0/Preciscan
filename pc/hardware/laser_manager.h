#pragma once
#include <string>
#include <vector>
#include "InterfaceLLT_2.h"
#include "../core/i_data_sink.h"

class LaserManager {
public:
    LaserManager(const std::string& dllPath = "LLT.dll", IDataSink* sink = nullptr);
    ~LaserManager();

    bool init();
    bool connect();
    void startAcquisition();
    void stopAcquisition();

    // SDK ConvertProfile2Values: ham bayt → mm (X=konum, Z=mesafe)
    // Cagiran: vector boyutlarini resolution() kadar ayirmalı
    bool convertProfile(const unsigned char* data, size_t size,
                        std::vector<double>& outX, std::vector<double>& outZ) const;

    // Sensör bilgisi (connect() sonrası geçerli)
    unsigned int  resolution()   const { return m_resolution; }
    TScannerType  scannerType()  const { return m_scannerType; }
    std::string   getLastError() const { return m_lastError; }

private:
    std::string   m_dllPath;
    std::string   m_lastError;
    CInterfaceLLT* m_llt        = nullptr;
    IDataSink*    m_sink        = nullptr;
    bool          m_connected   = false;
    unsigned int  m_resolution  = 0;
    TScannerType  m_scannerType = StandardType;
};
