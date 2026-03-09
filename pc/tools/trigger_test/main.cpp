// trigger_test/main.cpp
//
// Micro-Epsilon scanCONTROL trigger test tool
//
// Asama 1: Software trigger -- SDK'nin TriggerProfile() fonksiyonu ile tetikle
// Asama 2: Hardware trigger -- TRIG_EXT_ACTIVE modunda dis pin ile tetikle
// Asama 3: Frekans testi   -- N adet tetikleme, atlama var mi olc
//
// Kullanim:
//   trigger_test.exe --software          Asama 1: software trigger, 1 profil al
//   trigger_test.exe --hardware          Asama 2: hardware trigger, pin bekle
//   trigger_test.exe --freq <Hz> <count> Asama 3: <Hz> frekansta <count> tetikleme

#include "InterfaceLLT_2.h"

#include <chrono>
#include <conio.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

// ---------------------------------------------------------------------------
// Config
// ---------------------------------------------------------------------------

static const char*        kDllPath = "..\\LLT.dll";
static const unsigned int kMaxDev  = 6;
static const unsigned int kMaxRes  = 6;

// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------

static CInterfaceLLT* g_llt         = nullptr;
static unsigned int   g_resolution  = 0;
static TScannerType   g_scannerType = scanCONTROL2xxx;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static bool checkOk(int ret, const char* context)
{
    if (ret < GENERAL_FUNCTION_OK)
    {
        char err[256] = {};
        if (g_llt)
            g_llt->TranslateErrorValue(ret, err, sizeof(err));
        std::cerr << "[ERROR] " << context << " => " << ret << " " << err << "\n";
        return false;
    }
    return true;
}

// Profil buffer'indan timestamp ve ilk 5 noktayi yazdir
static void printProfileInfo(std::vector<unsigned char>& buf)
{
    unsigned char* ts = &buf[g_resolution * 64 - 16];
    double       tOpen = 0, tClose = 0;
    unsigned int profileCount = 0;
    g_llt->Timestamp2TimeAndCount(ts, &tOpen, &tClose, &profileCount);
    std::cout << "[INFO] ShutterOpen=" << tOpen << " ShutterClose=" << tClose
              << " ProfileCount=" << profileCount << "\n";

    std::vector<double> vX(g_resolution), vZ(g_resolution);
    g_llt->ConvertProfile2Values(buf.data(), g_resolution, PROFILE, g_scannerType,
                                 0, true, nullptr, nullptr, nullptr,
                                 vX.data(), vZ.data(), nullptr, nullptr);
    std::cout << "[INFO] Ilk 5 nokta:\n";
    for (int i = 0; i < 5 && i < (int)g_resolution; i++)
        std::cout << "  [" << i << "] X=" << vX[i] << " Z=" << vZ[i] << "\n";
}

// ---------------------------------------------------------------------------
// Connect -- sadece baglanti, trigger ayari yapmiyor (user mode korunuyor)
// ---------------------------------------------------------------------------

static bool connectDevice()
{
    bool loadError = false;
    g_llt          = new CInterfaceLLT(kDllPath, &loadError);
    if (loadError)
    {
        std::cerr << "[ERROR] LLT.dll yuklenemedi: " << kDllPath << "\n";
        return false;
    }

    if (!checkOk(g_llt->CreateLLTDevice(INTF_TYPE_ETHERNET), "CreateLLTDevice"))
        return false;

    unsigned int devList[kMaxDev] = {};
    int          nFound           = g_llt->GetDeviceInterfaces(devList, kMaxDev);
    std::cout << "[INFO] GetDeviceInterfaces = " << nFound << " cihaz\n";
    if (nFound < 1)
    {
        std::cerr << "[ERROR] Agda scanCONTROL bulunamadi\n";
        return false;
    }

    if (!checkOk(g_llt->SetDeviceInterface(devList[0], 0), "SetDeviceInterface"))
        return false;
    if (!checkOk(g_llt->Connect(), "Connect"))
        return false;

    g_llt->GetLLTType(&g_scannerType);
    std::cout << "[INFO] ScannerType = " << (int)g_scannerType << "\n";

    std::vector<DWORD> resolutions(kMaxRes);
    g_llt->GetResolutions(resolutions.data(), kMaxRes);
    g_resolution = resolutions[0];
    g_llt->SetResolution(g_resolution);
    std::cout << "[INFO] Resolution = " << g_resolution << "\n";

    // NOT: Burada SetFeature(TRIGGER, ...) YAPILMIYOR
    // Cihazin flash'taki user mode ayarlari korunuyor
    // Her asama kendi trigger modunu kendisi set ediyor

    return true;
}

static void disconnectDevice()
{
    if (g_llt)
    {
        g_llt->TransferProfiles(NORMAL_TRANSFER, false);
        g_llt->Disconnect();
        delete g_llt;
        g_llt = nullptr;
    }
}

// ---------------------------------------------------------------------------
// Asama 1 -- Software trigger (TRIG_INTERNAL + TriggerProfile())
// ---------------------------------------------------------------------------

static void runSoftwareTrigger()
{
    std::cout << "\n=== ASAMA 1: Software Trigger ===\n";

    if (!checkOk(g_llt->SetFeature(FEATURE_FUNCTION_TRIGGER, TRIG_INTERNAL), "SetFeature TRIG_INTERNAL"))
        return;
    if (!checkOk(g_llt->SetProfileConfig(PROFILE), "SetProfileConfig"))
        return;
    if (!checkOk(g_llt->SetFeature(FEATURE_FUNCTION_EXPOSURE_TIME, 100), "SetExposure"))
        return;
    if (!checkOk(g_llt->SetFeature(FEATURE_FUNCTION_IDLE_TIME, 500), "SetIdle"))
        return;

    Sleep(120);

    if (!checkOk(g_llt->TransferProfiles(NORMAL_TRANSFER, true), "TransferProfiles ON"))
        return;

    std::vector<unsigned char> buf(g_resolution * 64);
    bool received = false;
    int  attempts = 0;

    std::cout << "[INFO] Tetikleniyor (software)...\n";

    while (!received && attempts < 100)
    {
        int ret = g_llt->GetActualProfile(buf.data(), (unsigned int)buf.size(), PROFILE, nullptr);
        if (ret == ERROR_PROFTRANS_NO_NEW_PROFILE)
        {
            g_llt->TriggerProfile();
            Sleep(10);
            attempts++;
        }
        else if (ret == (int)buf.size())
        {
            received = true;
        }
        else
        {
            checkOk(ret, "GetActualProfile");
            break;
        }
    }

    if (received)
    {
        std::cout << "[OK] Profil alindi (" << attempts << " denemede)\n";
        printProfileInfo(buf);
    }
    else
    {
        std::cerr << "[FAIL] " << attempts << " denemede profil alinamadi\n";
    }

    g_llt->TransferProfiles(NORMAL_TRANSFER, false);
}

// ---------------------------------------------------------------------------
// Asama 2 -- Hardware trigger (TRIG_EXT_ACTIVE, pin tetiklemesi bekleniyor)
// ---------------------------------------------------------------------------

static void runHardwareTrigger()
{
    std::cout << "\n=== ASAMA 2: Hardware Trigger ===\n";
    std::cout << "[INFO] Cihaz TRIG_EXT_ACTIVE moduna aliniyor...\n";

    // External trigger modunu SDK ile set et
    // Configuration Tool ayarini eziyoruz ama bu dogru davranis:
    // Digital input / RS422 pin tetiklenince cihaz profil uretecek
    DWORD trigValue = TRIG_EXT_ACTIVE | TRIG_INPUT_DIGIN | TRIG_MODE_EDGE | TRIG_POLARITY_HIGH;
    if (!checkOk(g_llt->SetFeature(FEATURE_FUNCTION_TRIGGER, trigValue), "SetFeature TRIGGER"))
        return;
    if (!checkOk(g_llt->SetProfileConfig(PROFILE), "SetProfileConfig"))
        return;
    if (!checkOk(g_llt->SetFeature(FEATURE_FUNCTION_EXPOSURE_TIME, 100), "SetExposure"))
        return;
    if (!checkOk(g_llt->SetFeature(FEATURE_FUNCTION_IDLE_TIME, 500), "SetIdle"))
        return;

    // Parametreler oturana kadar bekle
    Sleep(120);

    std::cout << "[INFO] Trigger pin'ini hazirla (sari kablo GND'ye)\n";
    std::cout << "[INFO] Hazir olunca ENTER'a bas...\n";
    std::cin.get();

    if (!checkOk(g_llt->TransferProfiles(NORMAL_TRANSFER, true), "TransferProfiles ON"))
        return;

    std::cout << "[INFO] Hardware trigger bekleniyor (30 saniye)...\n";
    std::cout << "[INFO] Simdi pin'i tetikle!\n";

    std::vector<unsigned char> buf(g_resolution * 64);
    bool received = false;

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(30);

    while (!received && std::chrono::steady_clock::now() < deadline)
    {
        int ret = g_llt->GetActualProfile(buf.data(), (unsigned int)buf.size(), PROFILE, nullptr);
        if (ret == (int)buf.size())
        {
            received = true;
        }
        else if (ret != ERROR_PROFTRANS_NO_NEW_PROFILE)
        {
            checkOk(ret, "GetActualProfile");
            break;
        }
        Sleep(5);
    }

    if (received)
    {
        std::cout << "[OK] Hardware trigger ile profil alindi!\n";
        printProfileInfo(buf);
    }
    else
    {
        std::cerr << "[FAIL] 30 saniye icinde hardware trigger algilanamadi\n";
        std::cerr << "[HINT] Kontrol listesi:\n";
        std::cerr << "  - Sari kablo (RS422 RX+) GND'ye bagli mi?\n";
        std::cerr << "  - Configuration Tool'da Digital input mode: trigger mi?\n";
        std::cerr << "  - Digital input logic: low level (5V) ise pull-up var,\n";
        std::cerr << "    GND'ye cekmek tetikler (kisa devre degil)\n";
    }

    g_llt->TransferProfiles(NORMAL_TRANSFER, false);
}

// ---------------------------------------------------------------------------
// Asama 3 -- Frekans testi (software trigger ile)
// ---------------------------------------------------------------------------

static void runFrequencyTest(unsigned int freqHz, unsigned int count)
{
    std::cout << "\n=== ASAMA 3: Frekans Testi ===\n";
    std::cout << "[INFO] Hedef: " << freqHz << " Hz, " << count << " tetikleme\n";

    const unsigned int periodMs = 1000u / freqHz;
    std::cout << "[INFO] Periyot: " << periodMs << " ms\n";

    if (periodMs < 2)
        std::cout << "[WARN] " << freqHz << " Hz cok yuksek olabilir, atlamalar bekleniyor.\n";

    if (!checkOk(g_llt->SetFeature(FEATURE_FUNCTION_TRIGGER, TRIG_INTERNAL), "SetFeature TRIG_INTERNAL"))
        return;
    if (!checkOk(g_llt->SetProfileConfig(PROFILE), "SetProfileConfig"))
        return;
    if (!checkOk(g_llt->SetFeature(FEATURE_FUNCTION_EXPOSURE_TIME, 100), "SetExposure"))
        return;
    if (!checkOk(g_llt->SetFeature(FEATURE_FUNCTION_IDLE_TIME, 500), "SetIdle"))
        return;

    Sleep(120);

    if (!checkOk(g_llt->TransferProfiles(NORMAL_TRANSFER, true), "TransferProfiles ON"))
        return;

    std::vector<unsigned char> buf(g_resolution * 64);
    unsigned int received     = 0;
    unsigned int missed       = 0;
    unsigned int lastCount    = 0;
    bool         firstProfile = true;

    auto testStart = std::chrono::steady_clock::now();

    for (unsigned int i = 0; i < count; i++)
    {
        auto trigTime = std::chrono::steady_clock::now();

        g_llt->TriggerProfile();

        bool got      = false;
        auto deadline = trigTime + std::chrono::milliseconds(periodMs * 2 + 10);

        while (std::chrono::steady_clock::now() < deadline)
        {
            int ret = g_llt->GetActualProfile(buf.data(), (unsigned int)buf.size(), PROFILE, nullptr);
            if (ret == (int)buf.size())
            {
                got = true;
                received++;

                unsigned char* ts = &buf[g_resolution * 64 - 16];
                double         tOpen = 0, tClose = 0;
                unsigned int   profileCount = 0;
                g_llt->Timestamp2TimeAndCount(ts, &tOpen, &tClose, &profileCount);

                if (!firstProfile && profileCount != lastCount + 1)
                {
                    unsigned int gap = profileCount - lastCount - 1;
                    missed += gap;
                    std::cout << "[WARN] Atlama: beklenen=" << (lastCount + 1)
                              << " gelen=" << profileCount << " gap=" << gap << "\n";
                }
                lastCount    = profileCount;
                firstProfile = false;
                break;
            }
            Sleep(1);
        }

        if (!got)
        {
            missed++;
            std::cout << "[MISS] Tetikleme #" << i << " profil alinamadi\n";
        }

        auto nextTrig = trigTime + std::chrono::milliseconds(periodMs);
        std::this_thread::sleep_until(nextTrig);
    }

    auto   testEnd  = std::chrono::steady_clock::now();
    double elapsed  = std::chrono::duration<double>(testEnd - testStart).count();
    double actualHz = received / elapsed;

    std::cout << "\n--- Frekans Testi Sonuclari ---\n";
    std::cout << "Hedef frekans : " << freqHz << " Hz\n";
    std::cout << "Gercek frekans: " << actualHz << " Hz\n";
    std::cout << "Tetikleme     : " << count << "\n";
    std::cout << "Alinan profil : " << received << "\n";
    std::cout << "Kacirilan     : " << missed << "\n";
    std::cout << "Basari orani  : " << (100.0 * received / count) << " %\n";

    g_llt->TransferProfiles(NORMAL_TRANSFER, false);
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    std::cout << "=== Preciscan Trigger Test Tool ===\n";
    std::cout << "Kullanim:\n";
    std::cout << "  trigger_test.exe --software\n";
    std::cout << "  trigger_test.exe --hardware\n";
    std::cout << "  trigger_test.exe --freq <Hz> <count>\n\n";

    if (argc < 2)
    {
        std::cerr << "[ERROR] Arguman gerekli\n";
        return 1;
    }

    std::string mode = argv[1];

    if (!connectDevice())
    {
        std::cerr << "[ERROR] Cihaza baglanamadi\n";
        return 1;
    }

    if (mode == "--software")
        runSoftwareTrigger();
    else if (mode == "--hardware")
        runHardwareTrigger();
    else if (mode == "--freq" && argc >= 4)
    {
        unsigned int hz    = std::stoul(argv[2]);
        unsigned int count = std::stoul(argv[3]);
        runFrequencyTest(hz, count);
    }
    else
        std::cerr << "[ERROR] Gecersiz arguman\n";

    disconnectDevice();

    std::cout << "\nCikmak icin bir tusa basin...\n";
    while (!_kbhit()) {}

    return 0;
}
