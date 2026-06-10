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
#include "../utils/point_cloud_processor.h"
#include <QThread>

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
                m_originalCloud = cloud;
                m_cloudHistory.clear();
                emit historySizeChanged(0);
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
void ScanController::startScan(int direction)
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
    m_hasStartAngle = false;
    m_startAngle = 0.0f;

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
            QString cmd = (direction == 0) ? "START_CW" : "START_CCW";
            m_serialReader->sendCommand(cmd.toStdString());
            emit logMessage("SYS", "Arduino'ya " + cmd + " komutu gonderildi.");
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
    if (!m_lastCloud.isEmpty()) {
        addNewLayer(m_lastCloud, "Tarama Katmani");
        m_lastCloud.clear();
    }
    emit scanStopped();
}

// =====================================================================
// consumeHardwarePackets — Ana veri işleme döngüsü
// =====================================================================
void ScanController::consumeHardwarePackets()
{
    // Lazer olmasa bile tetikleri oku (log'a dusur / 360 deg kontrolu yap)
    if (!m_scanning)
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
                emit logMessage("MCU", QString("Tetik: seq=%1 aci=%2 deg")
                    .arg(evt.seq).arg(angleDeg, 0, 'f', 2));
                m_encoderAngles.push(angleDeg);
            }
        }
        
        // 2. Serial port üzerinden MCU
        if (m_serialReader) {
            // Text mesajlarini oku
            std::string msg;
            while (m_serialReader->tryGetMessage(msg)) {
                emit logMessage("MCU", QString::fromStdString(msg));
            }

            SerialTriggerReader::TriggerEvent evt;
            while (evtCount < MAX_EVENTS && m_serialReader->tryGetTriggerEvent(evt)) {
                ++evtCount;
                const float angleDeg = static_cast<float>(evt.angle_mdeg) / 1000.0f;
                emit mcuPacketReceived(evt.seq, angleDeg);
                
                if (!m_hasStartAngle) {
                    m_startAngle = angleDeg;
                    m_hasStartAngle = true;
                }
                
                m_encoderAngles.push(angleDeg);

                // 360 derece (tam tur) tamamlandiysa durdur
                float sweptAngle = std::abs(angleDeg - m_startAngle);
                if (sweptAngle >= 359.5f) {
                    stopScan();
                    return;
                }
            }
        }


        // Lazer bagli degilse profil eslestirme atlaniyor
        if (!m_ring || !m_laser)
            return;

        // Akilli eslestirme: Buffer'daki profilleri topla, son N tanesini
        // N bekleyen aciya dagit. Eski profilleri atla.
        //
        // Ornek: 50 profil, 3 aci → profil #48→aci1, #49→aci2, #50→aci3
        // Bu sayede her aci zamansal olarak en yakin profili alir.
        {
            const int numAngles = static_cast<int>(m_encoderAngles.size());
            if (numAngles == 0) { 
                // Eger okunmayi bekleyen bir aci yoksa, lazerden gelen surekli profiller
                // buffer'i doldurup tasirir. Bu nedenle buffer'i temizliyoruz.
                m_ring->clear();
                return; 
            }

            // 1. Ring buffer'i tamamen bosalt, tum profilleri topla
            std::vector<Packet> allProfiles;
            allProfiles.reserve(128);
            {
                Packet tmp;
                while (m_ring->try_pop(tmp)) {
                    allProfiles.push_back(std::move(tmp));
                }
            }

            // 2. Gecerli ve hatasiz profilleri ayikla ve donustur
            std::vector<QVector<QPointF>> validProfiles;
            validProfiles.reserve(allProfiles.size());
            static std::vector<double> vX, vZ;
            
            for (auto& pkt : allProfiles) {
                if (!pkt.data.empty() && m_laser->convertProfile(pkt.data.data(), pkt.data.size(), vX, vZ)) {
                    const unsigned int res = m_laser->resolution();
                    QVector<QPointF> profile;
                    profile.reserve(static_cast<int>(res));
                    for (unsigned int j = 0; j < res; ++j) {
                        if (vX[j] == 0.0 && vZ[j] == 0.0) continue;
                        profile.push_back(QPointF(vX[j], vZ[j]));
                    }
                    if (!profile.isEmpty()) {
                        validProfiles.push_back(std::move(profile));
                    }
                }
            }

            const int profileCount = static_cast<int>(validProfiles.size());
            if (profileCount == 0) { return; }

            // 3. Kac profili kullanacagimizi hesapla
            const int usable = (numAngles < profileCount) ? numAngles : profileCount;
            const int skip   = profileCount - usable;

            // 4. Son 'usable' profili, ilk 'usable' aciya esle
            for (int i = 0; i < usable; ++i) {
                const auto& profile = validProfiles[skip + i];
                float angleDeg = m_encoderAngles.front();
                m_encoderAngles.pop();

                publishProfileFrame(angleDeg, profile);

                // 360 derece kontrolu
                float sweptAngle = std::abs(angleDeg - m_startAngle);
                if (sweptAngle >= 359.5f) {
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

    // Saniyede 100 profil uretiliyor. Eger UI donarsa ve fps duserse MAX_PER_TICK 4 oldugunda
    // tampon sisecek ve profil kaybedilecektir. Bu yuzden bu degeri guvenli bir miktara cikartiyoruz.
    static constexpr int MAX_PER_TICK = 64;
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

    float theta_rad = thetaDeg * (3.14159265359f / 180.0f);
    float cosA = std::cos(theta_rad);
    float sinA = std::sin(theta_rad);
    float tableZ = m_dOffset;
    float zOffset = 3.5f;
    float lateralOffset = m_lOffset;

    for (const auto& p : profile) {
        float z = p.x() - zOffset;
        float r = tableZ - p.y();
        if (std::abs(r) < 0.05f) continue;
        float X = r * cosA - lateralOffset * sinA;
        float Y = r * sinA + lateralOffset * cosA;
        m_lastCloud.push_back(QVector3D(X, Y, z));
    }

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

void ScanController::setLastCloud(const QVector<QVector3D>& cloud)
{
    m_lastCloud = cloud;
    m_originalCloud = cloud;
    m_cloudHistory.clear();
    emit historySizeChanged(0);
}

void ScanController::applyFilterCylindrical(float radiusMm, float minZ, float maxZ)
{
    if (m_activeLayerId == -1 || !m_layers.contains(m_activeLayerId)) return;
    auto& layer = m_layers[m_activeLayerId];
    if (layer.points.isEmpty()) return;

    layer.history.push_back(layer.points);
    if (layer.history.size() > 5) {
        layer.history.pop_front();
    }
    emit historySizeChanged(layer.history.size());

    layer.points = core::PointCloudProcessor::filterCylindrical(layer.points, radiusMm, minZ, maxZ);
    emit pointCloudReady(layer.points);
    emit logMessage("SYS", QString("Silindir filtre uygulandi. Kalan nokta: %1").arg(layer.points.size()));
}

void ScanController::applyFilterStatistical(int meanK, float stdDevThresh)
{
    if (m_activeLayerId == -1 || !m_layers.contains(m_activeLayerId)) return;
    auto& layer = m_layers[m_activeLayerId];
    if (layer.points.isEmpty()) return;

    layer.history.push_back(layer.points);
    if (layer.history.size() > 5) {
        layer.history.pop_front();
    }
    emit historySizeChanged(layer.history.size());

    layer.points = core::PointCloudProcessor::filterStatisticalOutlier(layer.points, meanK, stdDevThresh);
    emit pointCloudReady(layer.points);
    emit logMessage("SYS", QString("SOR (Statistical) uygulandi. Kalan nokta: %1").arg(layer.points.size()));
}

void ScanController::applyFilterRadius(float radiusMm, int minNeighbors)
{
    if (m_activeLayerId == -1 || !m_layers.contains(m_activeLayerId)) return;
    auto& layer = m_layers[m_activeLayerId];
    if (layer.points.isEmpty()) return;

    layer.history.push_back(layer.points);
    if (layer.history.size() > 5) {
        layer.history.pop_front();
    }
    emit historySizeChanged(layer.history.size());

    layer.points = core::PointCloudProcessor::filterRadiusOutlier(layer.points, radiusMm, minNeighbors);
    emit pointCloudReady(layer.points);
    emit logMessage("SYS", QString("ROR (Radius) uygulandi. Kalan nokta: %1").arg(layer.points.size()));
}

void ScanController::applyFilterVoxelGrid(float leafSizeMm)
{
    if (m_activeLayerId == -1 || !m_layers.contains(m_activeLayerId)) return;
    auto& layer = m_layers[m_activeLayerId];
    if (layer.points.isEmpty()) return;

    layer.history.push_back(layer.points);
    if (layer.history.size() > 5) {
        layer.history.pop_front();
    }
    emit historySizeChanged(layer.history.size());

    layer.points = core::PointCloudProcessor::filterVoxelGrid(layer.points, leafSizeMm);
    emit pointCloudReady(layer.points);
    emit logMessage("SYS", QString("Voxel Grid (%1 mm) uygulandi. Kalan nokta: %2").arg(leafSizeMm).arg(layer.points.size()));
}

void ScanController::applyManualDeletion(const QVector<int>& indicesToRemove)
{
    if (m_activeLayerId == -1 || !m_layers.contains(m_activeLayerId)) return;
    auto& layer = m_layers[m_activeLayerId];
    if (layer.points.isEmpty() || indicesToRemove.isEmpty()) return;

    layer.history.push_back(layer.points);
    if (layer.history.size() > 5) {
        layer.history.pop_front();
    }
    emit historySizeChanged(layer.history.size());

    // Noktalari tersten silmek (index kaymasini önlemek icin)
    QVector<int> sortedIndices = indicesToRemove;
    std::sort(sortedIndices.begin(), sortedIndices.end(), std::greater<int>());

    for (int idx : sortedIndices) {
        if (idx >= 0 && idx < layer.points.size()) {
            layer.points.remove(idx);
        }
    }

    emit pointCloudReady(layer.points);
    emit logMessage("SYS", QString("%1 nokta manuel olarak silindi.").arg(indicesToRemove.size()));
}

void ScanController::undoLastFilter()
{
    if (m_activeLayerId == -1 || !m_layers.contains(m_activeLayerId)) return;
    auto& layer = m_layers[m_activeLayerId];
    
    if (layer.history.isEmpty()) return;

    layer.points = layer.history.last();
    layer.history.pop_back();
    emit historySizeChanged(layer.history.size());

    emit pointCloudReady(layer.points);
    emit logMessage("SYS", "Geri al (Undo) islemi yapildi.");
}

void ScanController::resetCloud()
{
    if (m_activeLayerId == -1 || !m_layers.contains(m_activeLayerId)) return;
    auto& layer = m_layers[m_activeLayerId];

    if (layer.originalPoints.isEmpty()) return;

    layer.history.push_back(layer.points);
    if (layer.history.size() > 5) {
        layer.history.pop_front();
    }
    emit historySizeChanged(layer.history.size());

    layer.points = layer.originalPoints;
    emit pointCloudReady(layer.points);
    emit logMessage("SYS", "Nokta bulutu orijinal haline donduruldu.");
}




const QVector<QVector3D>& ScanController::getLastCloud() const { return m_lastCloud; }

void ScanController::addNewLayer(const QVector<QVector3D>& points, const QString& name)
{
    ScanLayerData layer;
    layer.name = name.isEmpty() ? QString("Katman %1").arg(m_nextLayerId + 1) : name;
    layer.points = points;
    layer.originalPoints = points;
    layer.zOffsetMm = 0.0f;
    
    m_nextLayerId++;
    m_layers.insert(m_nextLayerId, layer);
    
    emit layersUpdated();
    setActiveLayer(m_nextLayerId);
}

void ScanController::setActiveLayer(int id)
{
    if (m_layers.contains(id) || id == -1) {
        m_activeLayerId = id;
        emit activeLayerChanged(m_activeLayerId);
        
        if (id != -1) {
            emit pointCloudReady(m_layers[id].points);
        } else {
            emit requestClearVisualizer();
        }
    }
}

void ScanController::deleteLayer(int id)
{
    if (m_layers.remove(id) > 0) {
        emit layersUpdated();
        if (m_activeLayerId == id) {
            if (!m_layers.isEmpty()) {
                setActiveLayer(m_layers.lastKey());
            } else {
                setActiveLayer(-1);
            }
        }
    }
}

void ScanController::setLayerName(int id, const QString& name)
{
    if (m_layers.contains(id)) {
        m_layers[id].name = name;
        emit layersUpdated();
    }
}

void ScanController::setLayerZOffset(int id, float mm)
{
    if (m_layers.contains(id)) {
        float diff = mm - m_layers[id].zOffsetMm;
        m_layers[id].zOffsetMm = mm;
        
        for (auto& p : m_layers[id].points) {
            p.setZ(p.z() + diff);
        }
        
        emit layersUpdated();
        if (m_activeLayerId == id) {
            emit pointCloudReady(m_layers[id].points);
        }
    }
}

void ScanController::clearHistory()
{
    if (m_activeLayerId != -1 && m_layers.contains(m_activeLayerId)) {
        m_layers[m_activeLayerId].history.clear();
    }
}

void ScanController::generateMeshAsync()
{
    if (m_activeLayerId == -1 || !m_layers.contains(m_activeLayerId)) return;
    
    const QVector<QVector3D>& currentCloud = m_layers[m_activeLayerId].points;
    if (currentCloud.isEmpty()) return;
    
    emit processingStarted("Hizli Yuzey Olusturuluyor...");
    disconnect(&m_watcher, &QFutureWatcher<QVector<QVector3D>>::finished, this, nullptr);
    connect(&m_watcher, &QFutureWatcher<QVector<QVector3D>>::finished, this, &ScanController::onMeshFinished);
    
    QFuture<QVector<QVector3D>> future = QtConcurrent::run(&core::PointCloudProcessor::generateCylindricalMesh, currentCloud, 720, 1000);
    m_watcher.setFuture(future);
}

void ScanController::onMeshFinished()
{
    QVector<QVector3D> mesh = m_watcher.result();
    disconnect(&m_watcher, &QFutureWatcher<QVector<QVector3D>>::finished, this, nullptr);
    // meshReady signal does not exist, emitting to viz? Wait, MainWindow connects meshLoaded?
    // Let's emit meshLoaded
    emit meshLoaded(mesh);
    emit processingFinished();
}

void ScanController::clearMesh()
{
    emit meshLoaded(QVector<QVector3D>());
}

void ScanController::mergeWithICPAsync(const QVector<QVector3D>& target, const QVector<QVector3D>& source, int icpMode)
{
    if (m_icpRunning) return;
    m_icpRunning = true;
    emit processingStarted("ICP Hizalama...");
    
    disconnect(&m_icpWatcher, &QFutureWatcher<QVector<QVector3D>>::finished, this, nullptr);
    connect(&m_icpWatcher, &QFutureWatcher<QVector<QVector3D>>::finished, this, &ScanController::onIcpFinished);
    
    QFuture<QVector<QVector3D>> future = QtConcurrent::run([target, source, icpMode]() -> QVector<QVector3D> {
        QMatrix4x4 transform = core::PointCloudProcessor::calculateICP(source, target, icpMode, 50, 1e-5f);
        QVector<QVector3D> transformedSource;
        transformedSource.reserve(source.size());
        
        QMatrix4x4 initialRot;
        if (icpMode == 1) {
            initialRot.scale(1.0f, -1.0f, -1.0f);
        } else if (icpMode == 2) {
            initialRot.rotate(90.0f, 1.0f, 0.0f, 0.0f);
        } else if (icpMode == 3) {
            initialRot.rotate(-90.0f, 1.0f, 0.0f, 0.0f);
        }
        transform = transform * initialRot;
        for (const auto& p : source) {
            transformedSource.push_back(transform.map(p));
        }
        QVector<QVector3D> merged = target;
        merged.append(transformedSource);
        return merged;
    });
    m_icpWatcher.setFuture(future);
}

void ScanController::onIcpFinished()
{
    m_icpRunning = false;
    QVector<QVector3D> merged = m_icpWatcher.result();
    disconnect(&m_icpWatcher, &QFutureWatcher<QVector<QVector3D>>::finished, this, nullptr);
    if (!merged.isEmpty()) {
        addNewLayer(merged, "ICP Birlesimi");
    }
    emit processingFinished();
}

void ScanController::mergeSelectedLayers(const QVector<int>& layerIds, const QString& mode)
{
    if (layerIds.size() < 2) return;
    
    if (mode.contains("ICP", Qt::CaseInsensitive)) {
        int icpMode = 0;
        if (mode.contains("Ters", Qt::CaseInsensitive)) icpMode = 1;
        else if (mode.contains("X+90", Qt::CaseInsensitive)) icpMode = 2;
        else if (mode.contains("X-90", Qt::CaseInsensitive)) icpMode = 3;

        QVector<QVector3D> target = m_layers[layerIds[0]].points;
        QVector<QVector3D> source = m_layers[layerIds[1]].points;
        mergeWithICPAsync(target, source, icpMode);
    } else {
        // Direct merge
        QVector<QVector3D> merged;
        for (int id : layerIds) {
            if (m_layers.contains(id)) {
                merged.append(m_layers[id].points);
            }
        }
        addNewLayer(merged, "Birlesik Katman");
    }
}

void ScanController::onFilterFinished()
{
    // Placeholder, actually handled inside the filter functions?
    // Filters modify the current layer
}

void ScanController::beginManualAlignment()
{
    if (m_activeLayerId == -1 || !m_layers.contains(m_activeLayerId)) return;
    auto& layer = m_layers[m_activeLayerId];
    m_manualAlignBaseCloud = layer.points; // Yedekle
}

void ScanController::updateManualAlignment(float tx, float ty, float tz, float rx, float ry, float rz)
{
    if (m_activeLayerId == -1 || !m_layers.contains(m_activeLayerId)) return;
    if (m_manualAlignBaseCloud.isEmpty()) return;

    QMatrix4x4 transform;
    transform.translate(tx, ty, tz);
    transform.rotate(rx, 1, 0, 0);
    transform.rotate(ry, 0, 1, 0);
    transform.rotate(rz, 0, 0, 1);

    auto& layer = m_layers[m_activeLayerId];
    layer.points.clear();
    layer.points.reserve(m_manualAlignBaseCloud.size());
    for (const auto& p : m_manualAlignBaseCloud) {
        layer.points.push_back(transform.map(p));
    }
    emit pointCloudReady(layer.points);
}

void ScanController::commitManualAlignment()
{
    if (m_activeLayerId == -1 || !m_layers.contains(m_activeLayerId)) return;
    auto& layer = m_layers[m_activeLayerId];
    
    // Gecmise ekle
    layer.history.push_back(m_manualAlignBaseCloud);
    if (layer.history.size() > 5) {
        layer.history.pop_front();
    }
    emit historySizeChanged(layer.history.size());
    
    m_manualAlignBaseCloud.clear();
    emit logMessage("SYS", "Manuel hizalama kaydedildi.");
}

void ScanController::cancelManualAlignment()
{
    if (m_activeLayerId == -1 || !m_layers.contains(m_activeLayerId)) return;
    auto& layer = m_layers[m_activeLayerId];
    
    // Geri al
    if (!m_manualAlignBaseCloud.isEmpty()) {
        layer.points = m_manualAlignBaseCloud;
        emit pointCloudReady(layer.points);
    }
    m_manualAlignBaseCloud.clear();
}

