#include "ScanController.hpp"

#include <QDebug>
#include <QFileInfo>
#include <cstring>
#include <vector>
#include <algorithm>

#include "../../core/packet.h"
#include "../../hardware/laser_manager.h"
#include "../../io/ply_writer.h"
#include "../../io/stl_loader.h"
#include "../../net/mcu_listener.h"
#include "../../net/spsc_ring_buffer.h"
#include "../../sim/laser_sim_worker.h"
#include "../../sim/mesh_slicer.h"

ScanController::ScanController(McuListener* mcu,
                               LaserManager* laser,
                               SPSCRingBuffer* ring,
                               QObject* parent)
    : QObject(parent), m_mcu(mcu), m_laser(laser), m_ring(ring)
{
    m_hwTimer = new QTimer(this);
    m_hwTimer->setInterval(0);
    connect(m_hwTimer, &QTimer::timeout, this, &ScanController::consumeHardwarePackets);
}

ScanController::~ScanController()
{
    if (m_simWorker) {
        m_simWorker->stop();
        m_simWorker->wait();
        delete m_simWorker;
        m_simWorker = nullptr;
    }
    
    if (m_serialReader) {
        m_serialReader->stop();
        delete m_serialReader;
        m_serialReader = nullptr;
    }
}

void ScanController::connectMcu()
{
    m_mcuConnected = true;
    emit mcuConnectionChanged(true);
}

void ScanController::disconnectMcu()
{
    m_mcuConnected = false;
    emit mcuConnectionChanged(false);
}

void ScanController::setSerialPort(const QString& portName)
{
    m_serialPort = portName;
    emit logMessage("SYS", QString("Serial port ayarlandi: %1").arg(portName));
}

void ScanController::connectMcuSerial()
{
    if (m_serialReader) {
        m_serialReader->stop();
        delete m_serialReader;
    }

    m_serialReader = new SerialTriggerReader(m_serialPort.toStdString(), 115200);
    if (m_serialReader->start()) {
        m_mcuConnected = true;
        emit mcuConnectionChanged(true);
        emit logMessage("OK", QString("MCU (Serial) baglandi: %1").arg(m_serialPort));
    } else {
        m_mcuConnected = false;
        emit mcuConnectionChanged(false);
        emit logMessage("ERR", QString("MCU (Serial) baglanamadi: %1").arg(m_serialPort));
        delete m_serialReader;
        m_serialReader = nullptr;
    }
}

void ScanController::disconnectMcuSerial()
{
    if (m_serialReader) {
        m_serialReader->stop();
        delete m_serialReader;
        m_serialReader = nullptr;
    }
    m_mcuConnected = false;
    emit mcuConnectionChanged(false);
    emit logMessage("SYS", "MCU (Serial) baglantisi kesildi.");
}

bool ScanController::validateLaserTiming(QString* errorMsg) const
{
    if (m_laserProfileRate <= 0) {
        if (errorMsg)
            *errorMsg = "Profil hizi sifirdan buyuk olmalidir.";
        return false;
    }

    const int periodUs = 1000000 / m_laserProfileRate;
    if (m_laserShutterUs >= periodUs) {
        if (errorMsg) {
            *errorMsg =
                QString("Pozlama suresi (%1 us), secilen profil hizinin periyodundan (%2 us) kucuk olmalidir.")
                    .arg(m_laserShutterUs)
                    .arg(periodUs);
        }
        return false;
    }

    return true;
}

void ScanController::connectLaser()
{
    if (!m_laser) {
        emit logMessage("ERR", "LaserManager mevcut degil!");
        return;
    }

    QString timingError;
    if (!validateLaserTiming(&timingError)) {
        emit logMessage("ERR", timingError);
        return;
    }

    if (!m_laser->init()) {
        emit logMessage("ERR", "Lazer SDK yuklenemedi (LLT.dll)! Lazer bagli mi?");
        return;
    }

    // UI/controller ayarlarini cihaz yoneticisine aktar
    m_laser->setProfileRateHz(m_laserProfileRate);
    m_laser->setExposureTimeUs(m_laserShutterUs);
    m_laser->setAutoExposure(m_laserAutoShutter);
    m_laser->setMeasuringField(m_laserMeasuringField.toStdString());
    m_laser->setPointsPerProfile(static_cast<unsigned int>(m_laserPointsPerProfile));

    // Lazer her zaman Internal modda calisir (kendi zamanlayicisiyla profil uretir).
    // Encoder eslestirme yazilimda yapilir — fiziksel tetik bagiantisina bagli degiliz.
    m_laser->setTriggerMode(LaserManager::TriggerMode::Internal);
    emit logMessage("SYS", "Lazer tetik: Internal (surekli profil akisi)");

    if (!m_laser->connect()) {
        emit logMessage("ERR",
                        QString("Lazere baglanilamadi! Detay: %1")
                            .arg(QString::fromStdString(m_laser->getLastError())));
        return;
    }

    // Acquisition'ı başlat — External Trigger modunda lazer sadece
    // Arduino'dan fiziksel tetik geldiğinde profil üretecek.
    m_laser->startAcquisition();

    m_isSimMode = false;
    m_laserConnected = true;

    emit isSimModeChanged(false);
    emit laserConnectionChanged(true);

    emit logMessage(
        "OK",
        QString("Lazer baglandi. rate=%1 Hz, shutter=%2 us, auto=%3, field=%4, points=%5")
            .arg(m_laserProfileRate)
            .arg(m_laserShutterUs)
            .arg(m_laserAutoShutter ? "on" : "off")
            .arg(m_laserMeasuringField)
            .arg(m_laserPointsPerProfile));
}

void ScanController::connectLaserSim(const QString& stlPath)
{
    if (m_scanning)
        return;

#ifdef _WIN32
    sim::Mesh mesh = sim::loadSTL(stlPath.toStdWString());
#else
    sim::Mesh mesh = sim::loadSTL(stlPath.toStdString());
#endif

    if (mesh.triangles.empty()) {
        emit logMessage("ERR", "STL yuklenemedi veya bos!");
        return;
    }

    m_isSimMode = true;
    m_stlPath = stlPath;
    emit isSimModeChanged(true);

    QVector<QVector3D> meshPoints;
    meshPoints.reserve(static_cast<int>(mesh.triangles.size() * 3));
    for (const auto& tri : mesh.triangles) {
        for (int i = 0; i < 3; ++i) {
            meshPoints.push_back(QVector3D(tri.v[i].x, tri.v[i].y, tri.v[i].z));
        }
    }

    emit meshLoaded(meshPoints);

    m_laserConnected = true;
    emit laserConnectionChanged(true);
    emit logMessage("OK", QString("Lazer simulasyonu hazir: %1").arg(QFileInfo(stlPath).fileName()));

    sim::SliceParams p;
    p.D_offset_mm = m_dOffset;
    p.deg_per_step = m_resolution;
    p.rps = m_rps;

    if (m_simWorker) {
        m_simWorker->stop();
        m_simWorker->wait();
        delete m_simWorker;
        m_simWorker = nullptr;
    }

    m_simWorker = new sim::LaserSimWorker(std::move(mesh), p, this);

    connect(m_simWorker,
            &sim::LaserSimWorker::profileReady,
            this,
            [this](float theta, const QVector<QPointF>& sensorData) {
                if (!sensorData.isEmpty()) {
                    publishProfileFrame(theta, sensorData);
                }
            });

    connect(m_simWorker, &sim::LaserSimWorker::progressUpdated, this, &ScanController::simProgressUpdated);

    connect(m_simWorker,
            &sim::LaserSimWorker::scanComplete,
            this,
            [this](const QVector<QVector3D>& cloud) {
                m_lastCloud = cloud;
                emit pointCloudReady(cloud);
                stopScan();
            });
}

void ScanController::disconnectLaser()
{
    if (m_laser) {
        m_laser->stopAcquisition();
        m_laser->disconnect();
    }

    m_laserConnected = false;
    emit laserConnectionChanged(false);
    emit logMessage("SYS", "Lazer baglantisi kesildi.");
}

// =====================================================================
// startScan — Doğru sıralama ile temiz başlangıç
// =====================================================================
void ScanController::startScan()
{
    if (m_scanning)
        return;

    // ── 1. Tüm eski verileri temizle ────────────────────────────
    while (!m_encoderAngles.empty()) m_encoderAngles.pop();

    if (m_ring)
        m_ring->clear();

    if (m_serialReader) {
        SerialTriggerReader::TriggerEvent sEvt;
        while (m_serialReader->tryGetTriggerEvent(sEvt)) {}
    }
    if (m_mcu) {
        McuListener::TriggerEvent uEvt;
        while (m_mcu->tryGetTriggerEvent(uEvt)) {}
    }

    emit logMessage("SYS", "Tetik kuyruklari ve ring buffer temizlendi.");
    emit requestClearVisualizer();

    m_scanning = true;
    m_lastCloud.clear();
    m_hwAngle = 0.0f;

    if (m_isSimMode && m_simWorker) {
        m_simWorker->start();
    } else if (m_ring && m_laser) {

        // ── 2. Ring buffer temizle (birikmiş eski profilleri at) ──
        m_ring->clear();

        // ── 3. Kısa bekleme + son temizlik ──────────────────────
        // Lazer Internal modda sürekli çalışıyor. Temizleme anında
        // gelen son birkaç profili de atalım.
        QThread::msleep(100);
        m_ring->clear();

        // ── 7. Arduino'ya START gönder (motor dönsün) ───────────
        if (m_serialReader) {
            m_serialReader->sendCommand("START");
            emit logMessage("SYS", "Arduino'ya START komutu gonderildi.");
        }

        m_hwTimer->start();

        if (m_triggerMode == ScanTriggerMode::TimeBased)
            emit logMessage("SYS", "Tarama basladi: Time-Based mod");
        else if (m_triggerMode == ScanTriggerMode::Encoder)
            emit logMessage("SYS", "Tarama basladi: Encoder Trigger mod — MCU tetik bekleniyor");
        else
            emit logMessage("SYS", "Tarama basladi: External Trigger mod — harici sinyal bekleniyor");
    }

    emit scanStarted();
}

// =====================================================================
// stopScan — Temiz kapanış
// =====================================================================
void ScanController::stopScan()
{
    if (!m_scanning)
        return;

    m_scanning = false;

    // Arduino'ya durdur komutu
    if (m_serialReader)
        m_serialReader->sendCommand("STOP");

    if (m_isSimMode && m_simWorker)
        m_simWorker->stop();

    m_hwTimer->stop();

    // Lazer Internal modda surekli calisiyor, acquisition durdurmuyoruz.
    // Sadece tamponlari bosalt.

    // Tamponları boşalt
    if (m_ring)
        m_ring->clear();
    if (m_serialReader) {
        SerialTriggerReader::TriggerEvent sEvt;
        while (m_serialReader->tryGetTriggerEvent(sEvt)) {}
    }
    if (m_mcu) {
        McuListener::TriggerEvent uEvt;
        while (m_mcu->tryGetTriggerEvent(uEvt)) {}
    }

    m_hwAngle = 0.0f;

    emit logMessage("SYS", "Tarama durduruldu — lazer ve tamponlar temizlendi.");
    emit scanStopped();
}

// =====================================================================
// consumeHardwarePackets — Ana veri işleme döngüsü
// =====================================================================
void ScanController::consumeHardwarePackets()
{
    if (!m_ring || !m_scanning || !m_laser)
        return;

    // ═══════════════════════════════════════════════════════════════
    // Enkoder / External Trigger modu
    // ═══════════════════════════════════════════════════════════════
    if (m_triggerMode != ScanTriggerMode::TimeBased) {
        static constexpr int MAX_EVENTS = 16;
        int evtCount = 0;
        
        // 1. UDP üzerinden MCU (eski yöntem, varsa)
        if (m_mcu) {
            McuListener::TriggerEvent evt;
            while (evtCount < MAX_EVENTS && m_mcu->tryGetTriggerEvent(evt)) {
                ++evtCount;
                const float angleDeg = static_cast<float>(evt.angle_mdeg) / 1000.0f;
                emit mcuPacketReceived(evt.seq, angleDeg);
                emit logMessage("MCU", QString("Sinyal Aliniyor... Tetik:%1 Aci:%2 deg")
                    .arg(evt.seq).arg(angleDeg, 0, 'f', 2));
                m_encoderAngles.push(angleDeg);
            }
        }
        
        // 2. Serial port üzerinden MCU (Arduino)
        if (m_serialReader) {
            SerialTriggerReader::TriggerEvent evt;
            while (evtCount < MAX_EVENTS && m_serialReader->tryGetTriggerEvent(evt)) {
                ++evtCount;
                const float angleDeg = static_cast<float>(evt.angle_mdeg) / 1000.0f;
                emit mcuPacketReceived(evt.seq, angleDeg);
                emit logMessage("MCU", QString("Sinyal Aliniyor... Tetik:%1 Aci:%2 deg")
                    .arg(evt.seq).arg(angleDeg, 0, 'f', 2));
                m_encoderAngles.push(angleDeg);
            }
        }
        // Akilli eslestirme: Buffer'daki profilleri topla, son N tanesini
        // N bekleyen aciya dagit. Eski profilleri atla.
        //
        // Ornek: 50 profil, 3 aci → profil #48→aci1, #49→aci2, #50→aci3
        // Bu sayede her aci zamansal olarak en yakin profili alir.
        {
            const int numAngles = static_cast<int>(m_encoderAngles.size());
            if (numAngles == 0) { return; }

            // 1. Ring buffer'i tamamen bosalt, tum profilleri topla
            std::vector<Packet> allProfiles;
            allProfiles.reserve(128);
            {
                Packet tmp;
                while (m_ring->try_pop(tmp)) {
                    allProfiles.push_back(std::move(tmp));
                }
            }

            const int profileCount = static_cast<int>(allProfiles.size());
            if (profileCount == 0) { return; }

            // 2. Kac profili kullanacagimizi hesapla
            const int usable = (numAngles < profileCount) ? numAngles : profileCount;
            const int skip   = profileCount - usable;

            // 3. Son 'usable' profili, ilk 'usable' aciya esle
            static std::vector<double> vX, vZ;
            for (int i = 0; i < usable; ++i) {
                Packet& pkt = allProfiles[skip + i];
                float angleDeg = m_encoderAngles.front();
                m_encoderAngles.pop();

                if (!pkt.data.empty()) {
                    if (m_laser->convertProfile(pkt.data.data(), pkt.data.size(), vX, vZ)) {
                        const unsigned int res = m_laser->resolution();
                        QVector<QPointF> profile;
                        profile.reserve(static_cast<int>(res));
                        for (unsigned int j = 0; j < res; ++j) {
                            if (vX[j] == 0.0 && vZ[j] == 0.0) continue;
                            profile.push_back(QPointF(vX[j], vZ[j]));
                        }
                        if (!profile.isEmpty()) {
                            publishProfileFrame(angleDeg, profile);
                        }
                    }
                }

                // 360 derece kontrolu
                if (angleDeg >= 359.5f || angleDeg <= -359.5f) {
                    stopScan();
                    return;
                }
            }
        }
        return;
    }

    // ═══════════════════════════════════════════════════════════════
    // Time-Based modu
    // ═══════════════════════════════════════════════════════════════
    if (m_serialReader) {
        SerialTriggerReader::TriggerEvent evt;
        while (m_serialReader->tryGetTriggerEvent(evt)) {
            m_hwAngle = static_cast<float>(evt.angle_mdeg) / 1000.0f;
        }
    }

    if (m_hwAngle >= 359.5f) {
        stopScan();
        return;
    }

    static constexpr int MAX_PER_TICK = 4;
    Packet pkt;
    int processed = 0;

    static std::vector<double> vX, vZ;

    while (processed < MAX_PER_TICK && m_ring->try_pop(pkt)) {
        ++processed;

        if (pkt.data.empty())
            continue;

        const bool ok = m_laser->convertProfile(pkt.data.data(), pkt.data.size(), vX, vZ);
        if (!ok)
            continue;

        const unsigned int res = m_laser->resolution();
        QVector<QPointF> profile;
        profile.reserve(static_cast<int>(res));

        for (unsigned int i = 0; i < res; ++i) {
            const double x = vX[i];
            const double z = vZ[i];

            if (x == 0.0 && z == 0.0)
                continue;

            profile.push_back(QPointF(x, z));
        }

        if (!profile.isEmpty()) {
            publishProfileFrame(m_hwAngle, profile);
            
            if (!m_serialReader) {
                m_hwAngle += m_resolution;
                if (m_hwAngle >= 360.0f) {
                    stopScan();
                    break;
                }
            }
        }
    }
}

void ScanController::onEncoderTrigger(float angleDeg)
{
    // Artik queue üzerinden eslestirme yapiliyor.
}

void ScanController::publishProfileFrame(float thetaDeg, const QVector<QPointF>& profile)
{
    if (profile.isEmpty())
        return;

    ScanProfileFrame frame;
    frame.profile = profile;
    frame.thetaDegree = thetaDeg;
    frame.layerIndex = 0;
    frame.direction = ScanDirection::Clockwise;

    emit profileFrameReceived(frame);
    emit simProfileReceived(thetaDeg, profile);
}

void ScanController::saveCurrentScan(const QString& path)
{
    if (m_lastCloud.isEmpty())
        return;

    io::writePLY(path, m_lastCloud);
    emit logMessage("OK", QString("Kaydedildi: %1").arg(path));
}

void ScanController::setDOffset(float mm)
{
    m_dOffset = mm;
    emit logMessage("SYS", QString("D offset guncellendi: %1 mm").arg(m_dOffset, 0, 'f', 2));

    if (m_isSimMode && !m_scanning)
        rebuildSimWorkerIfPossible();
}

void ScanController::setLateralOffset(float mm)
{
    m_lOffset = mm;
    emit logMessage("SYS", QString("Lateral offset guncellendi: %1 mm").arg(m_lOffset, 0, 'f', 2));

    if (m_isSimMode && !m_scanning)
        rebuildSimWorkerIfPossible();
}

void ScanController::setResolution(float val)
{
    m_resolution = val;
    emit logMessage("SYS", QString("Tarama cozunurlugu guncellendi: %1 deg").arg(m_resolution, 0, 'f', 2));

    if (m_isSimMode && !m_scanning)
        rebuildSimWorkerIfPossible();
}

void ScanController::setRps(float val)
{
    m_rps = val;
    emit logMessage("SYS", QString("Donus hizi guncellendi: %1 rps").arg(m_rps, 0, 'f', 2));

    if (m_isSimMode && !m_scanning)
        rebuildSimWorkerIfPossible();
}

void ScanController::setLaserProfileRate(int hz)
{
    m_laserProfileRate = hz;

    QString err;
    if (!validateLaserTiming(&err)) {
        emit logMessage("WARN", err);
        return;
    }

    emit logMessage("SYS", QString("Lazer profil hizi guncellendi: %1 Hz").arg(m_laserProfileRate));
}

void ScanController::setLaserShutterUs(int us)
{
    m_laserShutterUs = us;

    QString err;
    if (!validateLaserTiming(&err)) {
        emit logMessage("WARN", err);
        return;
    }

    emit logMessage("SYS", QString("Lazer shutter guncellendi: %1 us").arg(m_laserShutterUs));
}

void ScanController::setLaserAutoShutter(bool enabled)
{
    m_laserAutoShutter = enabled;
    emit logMessage("SYS", QString("Otomatik pozlama: %1").arg(enabled ? "acik" : "kapali"));
}

void ScanController::setLaserMeasuringField(const QString& field)
{
    m_laserMeasuringField = field;
    emit logMessage("SYS", QString("Olcum alani guncellendi: %1").arg(m_laserMeasuringField));
}

void ScanController::setLaserPointsPerProfile(int points)
{
    m_laserPointsPerProfile = points;
    emit logMessage("SYS", QString("Profil nokta sayisi guncellendi: %1").arg(m_laserPointsPerProfile));
}

void ScanController::setTriggerMode(ScanTriggerMode mode)
{
    if (m_triggerMode == mode)
        return;

    m_triggerMode = mode;

    static const char* names[] = {"Time-Based", "Encoder", "External Trigger"};
    emit logMessage("SYS",
                    QString("Tetik modu degistirildi: %1")
                        .arg(names[static_cast<int>(mode)]));
    emit triggerModeChanged(mode);
}

void ScanController::rebuildSimWorkerIfPossible()
{
    if (!m_isSimMode || m_stlPath.isEmpty())
        return;

    connectLaserSim(m_stlPath);
}

bool ScanController::sendSerialCommand(const QString& cmd)
{
    if (!m_serialReader)
    {
        emit logMessage("ERR", "Serial port bagli degil. Komut gonderilemedi.");
        return false;
    }

    bool ok = m_serialReader->sendCommand(cmd.toStdString());
    if (ok)
        emit logMessage("SYS", QString("Arduino'ya gonderildi: %1").arg(cmd));
    else
        emit logMessage("ERR", QString("Komut gonderilemedi: %1").arg(cmd));
    return ok;
}

void ScanController::sendZMove(float mm)
{
    sendSerialCommand(QString("ZMOVE:%1").arg(mm, 0, 'f', 1));
}

void ScanController::sendZHome()
{
    sendSerialCommand("ZHOME");
}

void ScanController::sendLinHome()
{
    sendSerialCommand("HOME");
}