#include "laser_manager.h"
#include <iostream>

// Lazerden veri geldiginde tetiklenecek olan Callback (Su an sadece izliyoruz)
// [NOT] pData ve pUserData Faz 3'te SPSCRingBuffer'a baglanacak
void _stdcall GlobalLaserCallback(const unsigned char* pData, unsigned int nSize, void* pUserData) {
    (void)pData;     // [DUZELTME C4100] Kullanilmayan parametre uyarisi kapatildi
    (void)pUserData; // [DUZELTME C4100] Kullanilmayan parametre uyarisi kapatildi
    std::cout << "\r[LAZER] Profil Yakalandi: " << nSize << " byte" << std::flush;
}

LaserManager::LaserManager(const std::string& dllPath)
    : m_dllPath(dllPath), m_llt(nullptr), m_connected(false) {}

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
        m_llt->RegisterCallback(STD_CALL, (void*)GlobalLaserCallback, nullptr);
        m_llt->TransferProfiles(NORMAL_TRANSFER, 1);
        std::cout << "[LAZER] Veri akisi baslatildi.\n";
    }
}

void LaserManager::stopAcquisition() {
    if (m_llt && m_connected) {
        m_llt->TransferProfiles((TTransferProfileType)0, 0);
        m_llt->Disconnect();
        m_connected = false;
    }
}
