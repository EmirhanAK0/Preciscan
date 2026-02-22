#pragma once
#include <string>
#include "InterfaceLLT_2.h"
#include "../core/i_data_sink.h"

class LaserManager {
public:
    // sink: profil verisi geldiginde nereye iletilecek (nullptr = sadece log)
    // Lifetime: sink, LaserManager'dan daha uzun yasasin
    LaserManager(const std::string& dllPath = "LLT.dll", IDataSink* sink = nullptr);
    ~LaserManager();
    bool init();
    bool connect();
    void startAcquisition();
    void stopAcquisition();

private:
    std::string m_dllPath;
    CInterfaceLLT* m_llt;
    IDataSink* m_sink;   // Sahiplik yok — disaridan verilir, disarida yonetilir
    bool m_connected;
};
