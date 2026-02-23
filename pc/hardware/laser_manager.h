#pragma once
#include <string>
#include "InterfaceLLT_2.h"
#include "../core/i_data_sink.h"

class LaserManager {
public:
    LaserManager(const std::string& dllPath = "LLT.dll", IDataSink* sink = nullptr);
    ~LaserManager();
    bool init();
    bool connect();       // Otomatik interface/index dener
    void startAcquisition();
    void stopAcquisition();
    std::string getLastError() const { return m_lastError; }

private:
    std::string m_dllPath;
    std::string m_lastError;
    CInterfaceLLT* m_llt;
    IDataSink* m_sink;
    bool m_connected;
};
