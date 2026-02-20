#pragma once
#include <string>
#include "InterfaceLLT_2.h"

class LaserManager {
public:
    LaserManager(const std::string& dllPath = "LLT.dll");
    ~LaserManager();
    bool init();
    bool connect();
    void startAcquisition();
    void stopAcquisition();

private:
    std::string m_dllPath;
    CInterfaceLLT* m_llt;
    bool m_connected;
};
