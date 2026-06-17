#include "ScanController.hpp"

#include <QDebug>
#include <QFileInfo>
#include <cstring>
#include <vector>
#include <algorithm>
#include <limits>

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
    m_calibrator = new AutoCalibrator(this);
    // m_calibrator->loadCalibration(); artik otomatik yuklemiyoruz

    connect(m_calibrator, &AutoCalibrator::calibrationFinished, this, [this](bool success, QMatrix4x4 mat, QString msg) {
        emit processingFinished();
        emit logMessage(success ? "OK" : "ERR", "3D Calibration: " + msg);
        
        if (success) {
            reapplyCurrentCalibration();
        }
        
        emit calibration3DFinished(success, msg);
    });

    m_hwTimer = new QTimer(this);
    m_hwTimer->setInterval(0);
    connect(m_hwTimer, &QTimer::timeout, this, &ScanController::consumeHardwarePackets);

    // Lazer baglanti watchdog'u: SDK Connect cagrisi (ag/IP sorununda Windows TCP
    // timeout'u ~20 sn surebilir) uzarsa kullaniciya 12 sn'de geri bildirim verip
    // butonu serbest birakir. Worker thread sonucu geldiginde temizlik yapilir.
    m_laserConnectTimeout = new QTimer(this);
    m_laserConnectTimeout->setSingleShot(true);
    m_laserConnectTimeout->setInterval(12000);
    connect(m_laserConnectTimeout, &QTimer::timeout, this, [this]() {
        if (!m_laserConnecting)
            return;
        m_laserConnectTimedOut = true;
        emit logMessage("WARN",
            "No laser connection response within 12 s. Are the device IP and your PC Ethernet "
            "adapter on the same subnet? (Background attempt still running.)");
        emit laserConnectFailed(
            "Timeout — device did not respond.\nAre the PC adapter and the laser on the same subnet?");
    });
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
        emit logMessage("OK", QString("MCU (Serial) connected: %1").arg(m_serialPort));
    } else {
        m_mcuConnected = false;
        emit mcuConnectionChanged(false);
        emit logMessage("ERR", QString("MCU (Serial) connection failed: %1").arg(m_serialPort));
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
    emit logMessage("SYS", "MCU (Serial) disconnected.");
}

bool ScanController::validateLaserTiming(QString* errorMsg) const
{
    if (m_laserProfileRate <= 0) {
        if (errorMsg)
            *errorMsg = "Profile rate must be greater than zero.";
        return false;
    }

    const int periodUs = 1000000 / m_laserProfileRate;
    if (m_laserShutterUs >= periodUs) {
        if (errorMsg) {
            *errorMsg =
                QString("Exposure time (%1 us) must be smaller than the selected profile rate period (%2 us).")
                    .arg(m_laserShutterUs)
                    .arg(periodUs);
        }
        return false;
    }

    return true;
}

void ScanController::connectLaser()
{
    // Zaten bir baglanti denemesi sürüyorsa tekrar baslatma (cift tiklamaya karsi).
    if (m_laserConnecting) {
        emit logMessage("SYS", "A laser connection attempt is already in progress, please wait.");
        return;
    }

    if (!m_laser) {
        emit logMessage("ERR", "LaserManager is not available!");
        emit laserConnectFailed("LaserManager is not available!");
        return;
    }

    QString timingError;
    if (!validateLaserTiming(&timingError)) {
        emit logMessage("ERR", timingError);
        emit laserConnectFailed(timingError);
        return;
    }

    // init() = DLL yukleme: hizli, UI thread'inde kalabilir.
    if (!m_laser->init()) {
        const QString msg = "Laser SDK could not be loaded (LLT.dll)! Is the laser connected?";
        emit logMessage("ERR", msg);
        emit laserConnectFailed(msg);
        return;
    }

    // UI/controller ayarlarini cihaz yoneticisine aktar (hizli setter'lar)
    m_laser->setProfileRateHz(m_laserProfileRate);
    m_laser->setExposureTimeUs(m_laserShutterUs);
    m_laser->setAutoExposure(m_laserAutoShutter);
    m_laser->setMeasuringField(m_laserMeasuringField.toStdString());
    m_laser->setPointsPerProfile(static_cast<unsigned int>(m_laserPointsPerProfile));

    // Tetikleme modunu aktar
    if (m_triggerMode == ScanTriggerMode::TimeBased) {
        m_laser->setTriggerMode(LaserManager::TriggerMode::Internal);
        emit logMessage("SYS", "Laser trigger: Internal (continuous profile stream)");
    } else {
        m_laser->setTriggerMode(LaserManager::TriggerMode::ExternalDigitalIn);
        emit logMessage("SYS", "Laser trigger: External/Encoder (waiting for hardware trigger)");
    }

    // ── Yavas kisim (cihaz kesfi + ag handshake + ayarlar) worker thread'de ──
    // Bu cagrilar USB->Ethernet ceviricide saniyeler surebilir; UI thread'inde
    // calistirilirsa arayuz donar. QtConcurrent ile thread havuzuna aliyoruz.
    m_laserConnecting = true;
    m_laserConnectTimedOut = false;
    emit laserConnectStarted();
    emit logMessage("SYS", "Connecting to laser... (in background)");
    m_laserConnectTimeout->start();

    disconnect(&m_laserConnectWatcher, &QFutureWatcher<LaserConnectResult>::finished, this, nullptr);
    connect(&m_laserConnectWatcher, &QFutureWatcher<LaserConnectResult>::finished, this,
            &ScanController::onLaserConnectFinished);

    LaserManager* laser = m_laser;
    QFuture<LaserConnectResult> future = QtConcurrent::run([laser]() -> LaserConnectResult {
        LaserConnectResult r;
        r.ok = laser->connect();
        if (!r.ok)
            r.error = QString::fromStdString(laser->getLastError());
        return r;
    });
    m_laserConnectWatcher.setFuture(future);
}

void ScanController::onLaserConnectFinished()
{
    disconnect(&m_laserConnectWatcher, &QFutureWatcher<LaserConnectResult>::finished, this, nullptr);
    m_laserConnectTimeout->stop();

    const LaserConnectResult r = m_laserConnectWatcher.result();
    const bool timedOut = m_laserConnectTimedOut;
    m_laserConnecting = false;
    m_laserConnectTimedOut = false;

    // Watchdog daha once devreye girip UI'a 'zaman asimi' bildirdiyse, worker
    // simdi (gec) dondu. UI'i tekrar 'basarisiz'la bombardiman etmeyiz.
    if (timedOut) {
        if (!r.ok) {
            const QString detail = r.error.isEmpty() ? QString("zaman asimi") : r.error;
            emit logMessage("ERR", QString("Laser connection attempt finished: %1").arg(detail));
            return;
        }
        // Gec gelen basari: cihaz aslinda baglandi. Tutarlilik icin kabul et.
        m_laser->startAcquisition();
        m_isSimMode = false;
        m_laserConnected = true;
        emit isSimModeChanged(false);
        emit laserConnectionChanged(true);
        emit logMessage("OK", "Laser connected (late response).");
        return;
    }

    if (!r.ok) {
        const QString detail = r.error.isEmpty() ? QString("unknown error") : r.error;
        emit logMessage("ERR", QString("Could not connect to laser! Detail: %1").arg(detail));
        emit laserConnectFailed(QString("Could not connect:\n%1").arg(detail));
        return;
    }

    // Baglanti tamam — acquisition'i baslat (hizli). connect() worker thread'de
    // bitti, bu slot UI thread'inde; ikisi es zamanli degil, yaris yok.
    // External Trigger modunda lazer sadece Arduino tetigi geldiginde profil uretir.
    m_laser->startAcquisition();

    m_isSimMode = false;
    m_laserConnected = true;

    emit isSimModeChanged(false);
    emit laserConnectionChanged(true);

    emit logMessage(
        "OK",
        QString("Laser connected. rate=%1 Hz, shutter=%2 us, auto=%3, field=%4, points=%5")
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
        emit logMessage("ERR", "STL could not be loaded or is empty!");
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
    p.deg_per_step = (m_triggerSource == TriggerSource::StepAngle) ? m_stepResolution : m_as5600Resolution;
    p.sec_per_rev = m_secPerRev;

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
    // Baglanti worker thread'de hala suruyorsa cihaza dokunma — es zamanli
    // erisim CInterfaceLLT'yi bozar. Buton UI'da kilitli oldugu icin normalde
    // buraya girilmez; yine de guvenlik amacli kontrol ediyoruz.
    if (m_laserConnecting) {
        emit logMessage("SYS", "A connection attempt is in progress; waiting for it to finish first.");
        return;
    }

    if (m_laser) {
        m_laser->stopAcquisition();
        m_laser->disconnect();
    }

    m_laserConnected = false;
    emit laserConnectionChanged(false);
    emit logMessage("SYS", "Laser disconnected.");
}

// =====================================================================
// startScan — Doğru sıralama ile temiz başlangıç
// =====================================================================
void ScanController::startScan(int direction)
{
    if (m_scanning)
        return;

    // ── 1. Tüm eski verileri temizle ────────────────────────────
    m_pendingTriggers.clear();
    m_pendingProfiles.clear();
    m_rawScanData.clear();

    m_statAngles = m_statProfiles = m_statEmptyProfiles = 0;
    m_statPaired = m_statSeqGaps = m_statDroppedProfiles = 0;
    m_hasLastSeq = false;
    m_lastSeq = 0;

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

    emit logMessage("SYS", "Trigger queues and ring buffer cleared.");
    emit requestClearVisualizer();

    m_scanning = true;
    m_lastCloud.clear();
    m_lastCloudRaw.clear();

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
            // Hiz ve Cozunurluk Ayarlarini Gonder
            uint32_t speed_us = (uint32_t)((m_secPerRev * 1000000.0f) / 3200.0f);
            if (speed_us < 200) speed_us = 200; // Guvenlik siniri
            
            float activeRes = (m_triggerSource == TriggerSource::StepAngle) ? m_stepResolution : m_as5600Resolution;
            int srcVal = static_cast<int>(m_triggerSource);

            m_serialReader->sendCommand(QString("TRIG_SRC:%1").arg(srcVal).toStdString());
            m_serialReader->sendCommand(QString("SPEED:%1").arg(speed_us).toStdString());
            m_serialReader->sendCommand(QString("RES:%1").arg(activeRes, 0, 'f', 2).toStdString());
            emit logMessage("SYS", QString("Motor settings sent: Revolution=%1 s (Step: %2 us), Resolution=%3, Source=%4")
                                       .arg(m_secPerRev).arg(speed_us).arg(activeRes)
                                       .arg(srcVal == 0 ? "AS5600" : "Step"));

            QString cmd = (direction == 0) ? "START_CW" : "START_CCW";
            m_serialReader->sendCommand(cmd.toStdString());
            emit logMessage("SYS", "To Arduino: " + cmd + " command sent.");
        }

        m_hwTimer->start();

        if (m_triggerMode == ScanTriggerMode::TimeBased)
            emit logMessage("SYS", "Scan started: Time-Based mode");
        else if (m_triggerMode == ScanTriggerMode::Encoder)
            emit logMessage("SYS", "Scan started: Encoder Trigger mode — waiting for MCU trigger");
        else
            emit logMessage("SYS", "Scan started: External Trigger mode — waiting for external signal");
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

    // ── Tani raporu: aci/profil sayilari uyusmuyorsa dalgalanmanin ──
    // ── kaynagi buyuk olasilikla tetik zinciridir; loga dusur.     ──
    if (m_triggerMode != ScanTriggerMode::TimeBased &&
        (m_statAngles > 0 || m_statProfiles > 0)) {
        emit logMessage("SYS",
            QString("Scan statistics: angle=%1, profiles=%2 (empty=%3), "
                    "matched=%4, seq loss=%5, overflow=%6, remaining in queue: angle=%7 profiles=%8")
                .arg(m_statAngles).arg(m_statProfiles).arg(m_statEmptyProfiles)
                .arg(m_statPaired).arg(m_statSeqGaps).arg(m_statDroppedProfiles)
                .arg(m_pendingTriggers.size()).arg(m_pendingProfiles.size()));

        if (m_statAngles != m_statProfiles) {
            emit logMessage("WARN",
                QString("Angle/profile counts are not equal (%1 != %2)! "
                        "Loss in the trigger chain; a likely source of surface waviness.")
                    .arg(m_statAngles).arg(m_statProfiles));
        }
    }
    m_pendingTriggers.clear();
    m_pendingProfiles.clear();

    emit logMessage("SYS", "Scan stopped — laser and buffers cleared.");
    if (!m_lastCloud.isEmpty()) {
        addNewLayer(m_lastCloud, "Scan Layer");
        // Ham veriyi de katmana kaydediyoruz: originalPoints (projekte edilmis,
        // kalibrasyonsuz) + rawProfiles (projeksiyon oncesi aci/profil ciftleri).
        if(m_layers.contains(m_activeLayerId)) {
            m_layers[m_activeLayerId].originalPoints = m_lastCloudRaw;
            m_layers[m_activeLayerId].rawProfiles = std::move(m_rawScanData);
        }
        m_lastCloud.clear();
        m_lastCloudRaw.clear();
        m_rawScanData.clear();
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
    // Enkoder / External Trigger modu — FIFO 1:1 eslestirme
    //
    // External trigger modunda her donanim tetigi tam olarak 1 profil
    // (Ethernet/ring buffer) ve 1 aci paketi (seri port) uretir. Iki akis
    // da kendi icinde siralidir; dogru eslestirme her zaman FIFO birebirdir.
    //
    // Eski "akilli eslestirme" iki sekilde veri bozuyordu:
    //  1. Aci kuyrugu bos oldugunda ring buffer temizleniyordu — seri port
    //     gecikmesi yuzunden acisi henuz gelmemis GECERLI profiller atiliyor,
    //     sonraki aci yanlis profile eslesiyordu (birikimli kayma → bulutta
    //     donmus hayalet kareler / yuzey dalgalanmasi).
    //  2. Profil fazlasi varsa EN ESKI profiller atlanip EN YENI profiller
    //     en eski acilara eslesiyordu — zamansal olarak ters.
    // ═══════════════════════════════════════════════════════════════
    if (m_triggerMode != ScanTriggerMode::TimeBased) {

        // ── 1. Tetik olaylarini oku (UDP + Serial) ──────────────────
        if (m_mcu) {
            McuListener::TriggerEvent evt;
            while (m_mcu->tryGetTriggerEvent(evt)) {
                const float angleDeg = static_cast<float>(evt.angle_mdeg) / 1000.0f;
                emit mcuPacketReceived(evt.seq, angleDeg);
                if (!m_hasStartAngle) {
                    m_startAngle = angleDeg;
                    m_hasStartAngle = true;
                }
                m_pendingTriggers.push_back({evt.seq, angleDeg});
                ++m_statAngles;
            }
        }

        if (m_serialReader) {
            // Text mesajlarini oku
            std::string msg;
            while (m_serialReader->tryGetMessage(msg)) {
                emit logMessage("MCU", QString::fromStdString(msg));
            }

            SerialTriggerReader::TriggerEvent evt;
            while (m_serialReader->tryGetTriggerEvent(evt)) {
                const float angleDeg = static_cast<float>(evt.angle_mdeg) / 1000.0f;
                emit mcuPacketReceived(evt.seq, angleDeg);

                // Seq surekliligi kontrolu: atlama = seri portta kaybolan paket.
                // (Eski ciplak-float formatta seq=0 gelir; o durumda kontrol atlanir.)
                if (evt.seq != 0) {
                    if (m_hasLastSeq && evt.seq > m_lastSeq + 1) {
                        m_statSeqGaps += (evt.seq - m_lastSeq - 1);
                        emit logMessage("WARN",
                            QString("Trigger packet loss: seq %1 -> %2")
                                .arg(m_lastSeq).arg(evt.seq));
                    }
                    m_lastSeq = evt.seq;
                    m_hasLastSeq = true;
                }

                if (!m_hasStartAngle) {
                    m_startAngle = angleDeg;
                    m_hasStartAngle = true;
                }

                m_pendingTriggers.push_back({evt.seq, angleDeg});
                ++m_statAngles;
            }
        }

        // Lazer bagli degilse profil eslestirme yapilamaz; sadece 360 kontrolu
        if (!m_ring || !m_laser) {
            if (!m_pendingTriggers.empty()) {
                const float lastAngle = m_pendingTriggers.back().angleDeg;
                if (std::abs(lastAngle - m_startAngle) >= 359.5f) {
                    stopScan();
                }
            }
            return;
        }

        // ── 2. Ring buffer'daki TUM profilleri sirayla kuyruga al ───
        // Donusturulemeyen/bos profil de kuyruga girer (bos QVector olarak):
        // o profil de bir tetigin sonucudur ve karsiligindaki aciyi tuketmesi
        // gerekir — aksi halde tum sonraki eslesmeler bir kayar.
        {
            Packet pkt;
            static std::vector<double> vX, vZ;
            while (m_ring->try_pop(pkt)) {
                QVector<QPointF> profile;
                if (!pkt.data.empty() &&
                    m_laser->convertProfile(pkt.data.data(), pkt.data.size(), vX, vZ)) {
                    const unsigned int res = m_laser->resolution();
                    profile.reserve(static_cast<int>(res));
                    for (unsigned int j = 0; j < res; ++j) {
                        if (vX[j] == 0.0 && vZ[j] == 0.0) continue;
                        profile.push_back(QPointF(vX[j], vZ[j]));
                    }
                }
                if (profile.isEmpty())
                    ++m_statEmptyProfiles;
                m_pendingProfiles.push_back(std::move(profile));
                ++m_statProfiles;
            }
        }

        // ── 3. Tasma korumasi ────────────────────────────────────────
        // Aci paketleri uzun sure gelmezse (seri kopmasi vb.) profil kuyrugu
        // sinirsiz buyumesin. Atilan her profil sayilir ve raporlanir.
        while (m_pendingProfiles.size() > 1024) {
            m_pendingProfiles.pop_front();
            ++m_statDroppedProfiles;
        }
        while (m_pendingTriggers.size() > 4096) {
            m_pendingTriggers.pop_front();
        }

        // ── 4. FIFO birebir eslestirme ───────────────────────────────
        while (!m_pendingTriggers.empty() && !m_pendingProfiles.empty()) {
            const PendingTrigger trig = m_pendingTriggers.front();
            m_pendingTriggers.pop_front();

            QVector<QPointF> profile = std::move(m_pendingProfiles.front());
            m_pendingProfiles.pop_front();

            ++m_statPaired;

            if (!profile.isEmpty())
                publishProfileFrame(trig.angleDeg, profile);

            // 360 derece (tam tur) kontrolu
            if (std::abs(trig.angleDeg - m_startAngle) >= 359.5f) {
                stopScan();
                return;
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
                float activeRes = (m_triggerSource == TriggerSource::StepAngle) ? m_stepResolution : m_as5600Resolution;
                m_hwAngle += activeRes;
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

void ScanController::projectProfileInto(float thetaDeg,
                                        const QVector<QPointF>& profile,
                                        QVector<QVector3D>& out) const
{
    const float theta_rad = thetaDeg * (3.14159265359f / 180.0f);
    const float cosA = std::cos(theta_rad);
    const float sinA = std::sin(theta_rad);
    const float tableZ = m_dOffset;
    const float zOffset = m_zBaseOffset;
    const float lateralOffset = m_lOffset;

    for (const auto& p : profile) {
        float z = p.x() - zOffset;
        float r = tableZ - p.y();
        if (std::abs(r) < 0.05f) continue;
        float X = r * cosA - lateralOffset * sinA;
        float Y = r * sinA + lateralOffset * cosA;
        out.push_back(QVector3D(X, Y, z));
    }
}

void ScanController::publishProfileFrame(float thetaDeg, const QVector<QPointF>& profile)
{
    if (profile.isEmpty())
        return;

    // Ham (aci, profil) ciftini sakla: offset/kalibrasyon sonradan degisirse
    // bulut bu veriden yeniden projekte edilir (yeniden tarama gerekmez).
    if (m_scanning) {
        RawProfileSample sample;
        sample.thetaDegree = thetaDeg;
        sample.profile = profile;
        m_rawScanData.push_back(std::move(sample));
    }

    ScanProfileFrame frame;
    frame.profile = profile;
    frame.thetaDegree = thetaDeg;
    frame.layerIndex = 0;
    frame.direction = ScanDirection::Clockwise;

    QVector<QVector3D> rawPoints;
    rawPoints.reserve(profile.size());
    projectProfileInto(thetaDeg, profile, rawPoints);

    const bool calibrated = (m_calibrator && m_calibrator->isCalibrated());
    for (const auto& p3d : rawPoints) {
        m_lastCloudRaw.push_back(p3d);
        m_lastCloud.push_back(calibrated ? m_calibrator->applyTransform(p3d) : p3d);
    }

    emit profileFrameReceived(frame);
    emit simProfileReceived(thetaDeg, profile);
}

void ScanController::reprojectActiveLayerFromRaw()
{
    if (m_activeLayerId == -1 || !m_layers.contains(m_activeLayerId))
        return;

    auto& layer = m_layers[m_activeLayerId];
    if (layer.rawProfiles.isEmpty())
        return;

    QVector<QVector3D> rawPoints;
    for (const auto& sample : layer.rawProfiles) {
        projectProfileInto(sample.thetaDegree, sample.profile, rawPoints);
    }

    // Katmana onceden verilmis Z kaydirmasini koru
    if (layer.zOffsetMm != 0.0f) {
        for (auto& p : rawPoints) {
            p.setZ(p.z() + layer.zOffsetMm);
        }
    }

    layer.originalPoints = rawPoints;

    if (m_calibrator && m_calibrator->isCalibrated()) {
        layer.points.clear();
        layer.points.reserve(rawPoints.size());
        for (const auto& p : rawPoints) {
            layer.points.push_back(m_calibrator->applyTransform(p));
        }
    } else {
        layer.points = rawPoints;
    }

    // Filtre gecmisi eski projeksiyona ait oldugu icin gecersiz
    layer.history.clear();
    emit historySizeChanged(0);

    emit layersUpdated();
    emit pointCloudReady(layer.points);
    emit logMessage("SYS",
        QString("Layer re-projected from raw data (dOffset=%1, lateral=%2). "
                "Applied filters have been reset.")
            .arg(m_dOffset, 0, 'f', 2).arg(m_lOffset, 0, 'f', 2));
}

void ScanController::saveCurrentScan(const QString& path)
{
    if (m_lastCloud.isEmpty())
        return;

    // Normalli kaydet: MeshLab'da normal hesaplama adimi atlanir,
    // dogrudan Poisson mesh kurulabilir.
    const QVector<QVector3D> normals = core::PointCloudProcessor::computeNormals(m_lastCloud);
    io::writePLYWithNormals(path, m_lastCloud, normals);
    emit logMessage("OK", QString("Kaydedildi (%1): %2")
                              .arg(normals.isEmpty() ? "normalsiz" : "normalli")
                              .arg(path));
}

void ScanController::setDOffset(float mm)
{
    const bool changed = (m_dOffset != mm);
    m_dOffset = mm;
    emit logMessage("SYS", QString("D offset updated: %1 mm").arg(m_dOffset, 0, 'f', 2));

    if (m_isSimMode && !m_scanning)
        rebuildSimWorkerIfPossible();

    // Yeni offset, aktif katmana ham veriden yeniden projeksiyon ile yansir
    if (changed && !m_scanning)
        reprojectActiveLayerFromRaw();
}

void ScanController::setLateralOffset(float mm)
{
    const bool changed = (m_lOffset != mm);
    m_lOffset = mm;
    emit logMessage("SYS", QString("Lateral offset updated: %1 mm").arg(m_lOffset, 0, 'f', 2));

    if (m_isSimMode && !m_scanning)
        rebuildSimWorkerIfPossible();

    // Yeni offset, aktif katmana ham veriden yeniden projeksiyon ile yansir
    if (changed && !m_scanning)
        reprojectActiveLayerFromRaw();
}

void ScanController::setZBaseOffset(float mm)
{
    const bool changed = (m_zBaseOffset != mm);
    m_zBaseOffset = mm;
    emit logMessage("SYS", QString("Height offset (Z0) updated: %1 mm").arg(m_zBaseOffset, 0, 'f', 2));
    emit zBaseOffsetChanged(m_zBaseOffset);

    if (changed && !m_scanning)
        reprojectActiveLayerFromRaw();
}

void ScanController::autoZeroBase(float maxRadiusMm)
{
    // Kaynak: aktif katmanin ham (kalibrasyonsuz) projeksiyonu
    QVector<QVector3D> cloud;
    if (m_activeLayerId != -1 && m_layers.contains(m_activeLayerId)) {
        const auto& layer = m_layers[m_activeLayerId];
        if (!layer.rawProfiles.isEmpty()) {
            for (const auto& sample : layer.rawProfiles) {
                projectProfileInto(sample.thetaDegree, sample.profile, cloud);
            }
        } else if (!layer.originalPoints.isEmpty()) {
            cloud = layer.originalPoints;
        }
    }
    if (cloud.isEmpty() && !m_lastCloudRaw.isEmpty()) {
        cloud = m_lastCloudRaw;
    }

    if (cloud.isEmpty()) {
        emit logMessage("WRN", "No scanned data found for base zeroing.");
        return;
    }

    // Eksene yakin noktalarin z degerlerini topla (dis ceper/tabla kenari haric)
    const float maxR2 = maxRadiusMm * maxRadiusMm;
    std::vector<float> zs;
    zs.reserve(cloud.size());
    for (const auto& p : cloud) {
        if (p.x() * p.x() + p.y() * p.y() <= maxR2) {
            zs.push_back(p.z());
        }
    }

    if (zs.size() < 200) {
        emit logMessage("WRN",
            QString("Base zeroing: not enough points within %1 mm of the axis (%2).")
                .arg(maxRadiusMm, 0, 'f', 0).arg(zs.size()));
        return;
    }

    // Robust taban tespiti: 2. yuzdelik dilim (alt kacak noktalara dayanikli),
    // ardindan bu seviyenin hemen ustundeki bandin medyani (tabla duzlemi).
    const size_t kLow = zs.size() / 50; // %2
    std::nth_element(zs.begin(), zs.begin() + kLow, zs.end());
    const float zLow = zs[kLow];

    std::vector<float> band;
    band.reserve(zs.size() / 10);
    for (float z : zs) {
        if (z >= zLow - 0.3f && z <= zLow + 1.0f) {
            band.push_back(z);
        }
    }
    float zBase = zLow;
    if (band.size() >= 50) {
        std::nth_element(band.begin(), band.begin() + band.size() / 2, band.end());
        zBase = band[band.size() / 2];
    }

    emit logMessage("SYS",
        QString("Base detected: z=%1 mm (%2 points). Correcting Z0 offset.")
            .arg(zBase, 0, 'f', 2).arg(band.size()));

    // z = p.x() - zBaseOffset → tabani 0'a cekmek icin ofseti zBase kadar artir
    setZBaseOffset(m_zBaseOffset + zBase);
}

void ScanController::setAs5600Resolution(float val)
{
    m_as5600Resolution = val;
    emit logMessage("SYS", QString("AS5600 resolution updated: %1 deg").arg(m_as5600Resolution, 0, 'f', 2));

    if (m_isSimMode && !m_scanning)
        rebuildSimWorkerIfPossible();
}

void ScanController::setStepResolution(float val)
{
    m_stepResolution = val;
    emit logMessage("SYS", QString("Step resolution updated: %1 deg").arg(m_stepResolution, 0, 'f', 2));

    if (m_isSimMode && !m_scanning)
        rebuildSimWorkerIfPossible();
}

void ScanController::setSecPerRev(float val)
{
    m_secPerRev = val;
    emit logMessage("SYS", QString("Revolution time updated: %1 s").arg(m_secPerRev, 0, 'f', 1));

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

    emit logMessage("SYS", QString("Laser profile rate updated: %1 Hz").arg(m_laserProfileRate));
}

void ScanController::setLaserShutterUs(int us)
{
    m_laserShutterUs = us;

    QString err;
    if (!validateLaserTiming(&err)) {
        emit logMessage("WARN", err);
        return;
    }

    if (m_laser && m_laser->isConnected()) {
        m_laser->setExposureTimeUs(m_laserShutterUs);
        m_laser->applyAcquisitionSettings();
    }

    emit logMessage("SYS", QString("Laser shutter updated: %1 us").arg(m_laserShutterUs));
}

void ScanController::setLaserAutoShutter(bool enabled)
{
    m_laserAutoShutter = enabled;
    
    if (m_laser && m_laser->isConnected()) {
        m_laser->setAutoExposure(m_laserAutoShutter);
        m_laser->applyAcquisitionSettings();
    }
    
    emit logMessage("SYS", QString("Otomatik pozlama: %1").arg(enabled ? "acik" : "kapali"));
}

void ScanController::setLaserMeasuringField(const QString& field)
{
    m_laserMeasuringField = field;
    emit logMessage("SYS", QString("Measuring field updated: %1").arg(m_laserMeasuringField));
}

void ScanController::setLaserPointsPerProfile(int points)
{
    m_laserPointsPerProfile = points;
    emit logMessage("SYS", QString("Points per profile updated: %1").arg(m_laserPointsPerProfile));
}

void ScanController::setTriggerMode(ScanTriggerMode mode)
{
    if (m_triggerMode == mode)
        return;

    m_triggerMode = mode;

    static const char* names[] = {"Time-Based", "Encoder", "External Trigger"};
    emit logMessage("SYS",
                    QString("Trigger mode changed: %1")
                        .arg(names[static_cast<int>(mode)]));

    if (m_laser && m_laser->isConnected()) {
        if (m_triggerMode == ScanTriggerMode::TimeBased) {
            m_laser->setTriggerMode(LaserManager::TriggerMode::Internal);
        } else {
            m_laser->setTriggerMode(LaserManager::TriggerMode::ExternalDigitalIn);
        }
        m_laser->applyAcquisitionSettings();
    }

    emit triggerModeChanged(mode);
}

void ScanController::setTriggerSource(TriggerSource src)
{
    if (m_triggerSource == src)
        return;

    m_triggerSource = src;
    emit logMessage("SYS", QString("Trigger source changed: %1")
                               .arg(src == TriggerSource::AS5600 ? "AS5600" : "Step Acisi"));

    if (m_isSimMode && !m_scanning)
        rebuildSimWorkerIfPossible();
    
    emit triggerSourceChanged(src);
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
        emit logMessage("ERR", "Serial port not connected. Command could not be sent.");
        return false;
    }

    bool ok = m_serialReader->sendCommand(cmd.toStdString());
    if (ok)
        emit logMessage("SYS", QString("Sent to Arduino: %1").arg(cmd));
    else
        emit logMessage("ERR", QString("Command could not be sent: %1").arg(cmd));
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
    emit logMessage("SYS", QString("Cylinder filter applied. Remaining points: %1").arg(layer.points.size()));
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
    emit logMessage("SYS", QString("SOR (Statistical) applied. Remaining points: %1").arg(layer.points.size()));
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
    emit logMessage("SYS", QString("ROR (Radius) applied. Remaining points: %1").arg(layer.points.size()));
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
    emit logMessage("SYS", QString("Voxel Grid (%1 mm) applied. Remaining points: %2").arg(leafSizeMm).arg(layer.points.size()));
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
    emit logMessage("SYS", QString("%1 points deleted manually.").arg(indicesToRemove.size()));
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
    emit logMessage("SYS", "Point cloud reverted to its original state.");
}




const QVector<QVector3D>& ScanController::getLastCloud() const { return m_lastCloud; }

void ScanController::addNewLayer(const QVector<QVector3D>& points, const QString& name)
{
    ScanLayerData layer;
    layer.name = name.isEmpty() ? QString("Layer %1").arg(m_nextLayerId + 1) : name;
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
    
    emit processingStarted("Generating Quick Surface...");
    disconnect(&m_watcher, &QFutureWatcher<QVector<QVector3D>>::finished, this, nullptr);
    connect(&m_watcher, &QFutureWatcher<QVector<QVector3D>>::finished, this, &ScanController::onMeshFinished);
    
    QFuture<QVector<QVector3D>> future = QtConcurrent::run(&core::PointCloudProcessor::generateCylindricalMesh, currentCloud, 720, 1000);
    m_watcher.setFuture(future);
}

void ScanController::start3DCalibrationAsync(int method)
{
    QVector<QVector3D> targetCloud = m_lastCloud;
    if (targetCloud.isEmpty()) {
        if (m_layers.contains(m_activeLayerId)) {
            targetCloud = m_layers[m_activeLayerId].points;
        } else if (!m_layers.isEmpty()) {
            targetCloud = m_layers.last().points;
        }
    }

    if (targetCloud.isEmpty()) {
        emit logMessage("WARN", "No point cloud for calibration. Please scan the calibration set first.");
        return;
    }
    emit processingStarted("Running 3D PCL Calibration...");
    if (m_calibrator) {
        CalibrationMethod calMethod = (method == 1) ? CalibrationMethod::MATH_PCA : CalibrationMethod::PCL_RANSAC;
        m_calibrator->startCalibration(targetCloud, calMethod);
    }
}

void ScanController::reapplyCurrentCalibration()
{
    // Mevcut butun layer'lari orijinal noktalar uzerinden tekrar hesapla
    for (auto it = m_layers.begin(); it != m_layers.end(); ++it) {
        if (!it->originalPoints.isEmpty()) {
            it->points.clear();
            for (const auto& p : it->originalPoints) {
                it->points.push_back(m_calibrator->applyTransform(p));
            }
        }
    }
    // m_lastCloud (su an devam eden tarama)
    if (!m_lastCloudRaw.isEmpty()) {
        m_lastCloud.clear();
        for (const auto& p : m_lastCloudRaw) {
            m_lastCloud.push_back(m_calibrator->applyTransform(p));
        }
    }
    emit layersUpdated();
    
    // Ekrani hemen guncelle
    if (m_activeLayerId != -1 && m_layers.contains(m_activeLayerId)) {
        emit pointCloudReady(m_layers[m_activeLayerId].points);
    } else if (!m_lastCloud.isEmpty()) {
        emit pointCloudReady(m_lastCloud);
    }
}

void ScanController::updateActiveCalibration(QString filePath)
{
    if (m_calibrator && m_calibrator->loadCalibration(filePath)) {
        emit logMessage("OK", QString("Calibration loaded: %1").arg(filePath));
        reapplyCurrentCalibration();
    } else {
        emit logMessage("ERR", "Calibration file could not be loaded!");
    }
}

void ScanController::disableCalibration()
{
    if (m_calibrator) {
        m_calibrator->clearCalibration();
        emit logMessage("SYS", "3D Calibration disabled. Reverted to raw data.");
        
        // Mevcut butun layer'lari eski haline (ham) cevir
        for (auto it = m_layers.begin(); it != m_layers.end(); ++it) {
            if (!it->originalPoints.isEmpty()) {
                it->points = it->originalPoints;
            }
        }
        if (!m_lastCloudRaw.isEmpty()) {
            m_lastCloud = m_lastCloudRaw;
        }
        emit layersUpdated();
    }
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
    emit processingStarted("Alignment + Merge...");
    emit logMessage("SYS", "Merge pipeline started: coarse alignment (table constraint + yaw) -> GICP -> fusion");

    disconnect(&m_icpWatcher, &QFutureWatcher<core::MergeResult>::finished, this, nullptr);
    connect(&m_icpWatcher, &QFutureWatcher<core::MergeResult>::finished, this, &ScanController::onIcpFinished);

    QFuture<core::MergeResult> future = QtConcurrent::run([target, source, icpMode]() -> core::MergeResult {
        core::MergeResult mr;

        // 1) Kaba hizalama adaylari.
        //    icpMode 4 (Mevcut Konumdan): kullanici katmani elle hizalamistir;
        //    preset/yaw HICBIR sey uygulanmaz, mevcut pozdan dogrudan GICP.
        //    Diger modlarda: preset + tabla kisiti + yaw taramasinin en iyi 3
        //    yerel minimumu aday olarak alinir (simetrik objelerde kaba skor
        //    yanlis 90 derece katina kilitlenebilir; secimi GICP yapar).
        QVector<QMatrix4x4> candidates;
        if (icpMode == 4) {
            candidates.push_back(QMatrix4x4());
        } else {
            candidates = core::PointCloudProcessor::coarseAlignCandidates(
                source, target, icpMode, 3);
        }

        // 2) Eleme turu: her aday hizli GICP'den gecirilir (1.0 mm voxel,
        //    25 iterasyon); en yuksek ortusme (sıkı 0.5 mm esik) kazanir.
        QMatrix4x4 bestGuess = candidates.first();
        if (candidates.size() > 1) {
            float bestOverlap = -1.0f;
            float bestRms = std::numeric_limits<float>::max();
            for (const auto& cand : candidates) {
                const core::AlignResult quick = core::PointCloudProcessor::refineAlignGICP(
                    source, target, cand, /*voxelMm=*/1.0f, /*maxIterations=*/25);
                const float ov = quick.success ? quick.overlapRatio : -1.0f;
                if (ov > bestOverlap + 0.02f ||
                    (std::abs(ov - bestOverlap) <= 0.02f && quick.rmsMm < bestRms)) {
                    bestOverlap = ov;
                    bestRms = quick.rmsMm;
                    bestGuess = quick.success ? quick.transform : cand;
                }
            }
        }

        // 3) Ince hizalama: kazanan adaydan tam cozunurluklu iki asamali GICP
        const core::AlignResult fine = core::PointCloudProcessor::refineAlignGICP(
            source, target, bestGuess);

        // 4) Donusumu TAM cozunurluklu kaynaga uygula ve birlestir
        QVector<QVector3D> merged = target;
        merged.reserve(target.size() + source.size());
        for (const auto& p : source) {
            merged.push_back(fine.transform.map(p));
        }

        // 5) Fuzyon: cift cidari tek yuzeye erit (voxel-avg + MLS + SOR)
        mr.cloud = core::PointCloudProcessor::fuseClouds(merged, 0.25f, 1.2f);
        mr.rmsMm = fine.rmsMm;
        mr.overlapRatio = fine.overlapRatio;
        mr.icpSuccess = fine.success;
        return mr;
    });
    m_icpWatcher.setFuture(future);
}

void ScanController::onIcpFinished()
{
    m_icpRunning = false;
    const core::MergeResult mr = m_icpWatcher.result();
    disconnect(&m_icpWatcher, &QFutureWatcher<core::MergeResult>::finished, this, nullptr);

    if (!mr.cloud.isEmpty()) {
        emit logMessage(mr.icpSuccess ? "OK" : "WARN",
            QString("Merge complete: RMS=%1 mm, overlap=%2%, GICP=%3, points=%4")
                .arg(mr.rmsMm, 0, 'f', 3)
                .arg(mr.overlapRatio * 100.0f, 0, 'f', 1)
                .arg(mr.icpSuccess ? "yakinsadi" : "DID NOT CONVERGE (coarse alignment used)")
                .arg(mr.cloud.size()));

        if (mr.icpSuccess && mr.overlapRatio < 0.15f) {
            emit logMessage("WARN",
                "Overlap ratio is low (<%15). There may not be enough common surface "
                "between scans; if alignment is unreliable, try a different orientation.");
        }
        if (mr.icpSuccess && mr.rmsMm > 0.3f) {
            emit logMessage("WARN",
                "Alignment RMS is high (>0.3 mm). Verify the dOffset/lateral calibration; "
                "rigid alignment cannot fix shape distortion from calibration.");
        }

        addNewLayer(mr.cloud,
            QString("Birlesim (RMS %1 mm, %2%)")
                .arg(mr.rmsMm, 0, 'f', 2)
                .arg(mr.overlapRatio * 100.0f, 0, 'f', 0));
    } else {
        emit logMessage("ERR", "Merge failed: result cloud is empty.");
    }
    emit processingFinished();
}

void ScanController::mergeSelectedLayers(const QVector<int>& layerIds, const QString& mode)
{
    if (layerIds.size() < 2) return;
    
    if (mode.contains("ICP", Qt::CaseInsensitive)) {
        int icpMode = 0;
        if (mode.contains("Current", Qt::CaseInsensitive)) icpMode = 4;
        else if (mode.contains("Inverted", Qt::CaseInsensitive)) icpMode = 1;
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
        addNewLayer(merged, "Merged Layer");
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
    emit logMessage("SYS", "Manual alignment saved.");
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

#include <QtConcurrent>
#include "../utils/CalibrationEngine.hpp"

void ScanController::autoCalibrateOffsetAsync()
{
    // 1. Tercih: ham (aci, profil) verisi — dogrudan projeksiyonla kesin arama.
    QVector<RawProfileSample> rawCopy;
    if (!m_rawScanData.isEmpty()) {
        rawCopy = m_rawScanData;
    } else if (m_activeLayerId != -1 && m_layers.contains(m_activeLayerId)
               && !m_layers[m_activeLayerId].rawProfiles.isEmpty()) {
        rawCopy = m_layers[m_activeLayerId].rawProfiles;
    }

    // 2. Fallback: ham veri yoksa KALIBRE EDILMEMIS bulut uzerinden eski yontem.
    //    (m_lastCloud kalibre edilmis olabilir; geri-hesaplama ham projeksiyon
    //    varsayimina dayandigi icin kalibre bulutla yanlis sonuc verir.)
    QVector<QVector3D> cloudCopy;
    if (rawCopy.isEmpty()) {
        if (!m_lastCloudRaw.isEmpty()) {
            cloudCopy = m_lastCloudRaw;
        } else if (m_activeLayerId != -1 && m_layers.contains(m_activeLayerId)) {
            cloudCopy = m_layers[m_activeLayerId].originalPoints;
        }
    }

    if (rawCopy.isEmpty() && cloudCopy.isEmpty()) {
        emit logMessage("WRN", "No active scanned data or selected layer found for auto calibration.");
        return;
    }

    emit processingStarted("Otomatik Offset Hesaplaniyor");

    const float currentOffset = m_lOffset;
    const float dOffset = m_dOffset;
    const bool useRaw = !rawCopy.isEmpty();

    emit logMessage("SYS", useRaw
        ? "Offset search is running on raw (angle+profile) data."
        : "No raw data; offset search runs on the projected cloud (less precise).");

    auto future = QtConcurrent::run(
        [rawCopy, cloudCopy, currentOffset, dOffset, useRaw]() -> AutoCalibResult {
            if (useRaw) {
                return CalibrationEngine::findOptimalLateralOffsetFromRaw(
                    rawCopy, dOffset, -2.0f, 2.0f, 0.05f);
            }
            return CalibrationEngine::findOptimalLateralOffset(
                cloudCopy, currentOffset, -2.0f, 2.0f, 0.05f);
        });

    auto watcher = new QFutureWatcher<AutoCalibResult>(this);
    connect(watcher, &QFutureWatcher<AutoCalibResult>::finished, this, [this, watcher]() {
        AutoCalibResult res = watcher->result();
        emit processingFinished();
        
        if (res.success) {
            emit logMessage("SYS", QString("Auto Calibration successful. Best offset: %1 mm (Score: %2)").arg(res.bestLateralOffset, 0, 'f', 2).arg(res.maxScore, 0, 'f', 0));
            emit autoCalibrationFinished(res.bestLateralOffset, res.maxScore);
        } else {
            emit logMessage("ERR", "Auto Calibration failed.");
        }
        watcher->deleteLater();
    });

    watcher->setFuture(future);
}

void ScanController::calibrateDiameterAsync(float knownDiameterMm, float zMinMm, float zMaxMm,
                                            float maxRadiusMm)
{
    if (knownDiameterMm <= 0.0f || zMaxMm <= zMinMm) {
        emit logMessage("ERR", "Diameter calibration: invalid parameters.");
        return;
    }

    // Kaynak veri: KALIBRE EDILMEMIS (ham projeksiyon) bulut gerekir, cunku
    // dOffset duzeltmesi ham projeksiyon formulune (r = dOffset - y) aittir.
    // Tercih sirasi: aktif katmanin ham profillerinden guncel parametrelerle
    // taze projeksiyon → katmanin originalPoints'i → m_lastCloudRaw.
    QVector<QVector3D> cloudCopy;
    if (m_activeLayerId != -1 && m_layers.contains(m_activeLayerId)) {
        const auto& layer = m_layers[m_activeLayerId];
        if (!layer.rawProfiles.isEmpty()) {
            for (const auto& sample : layer.rawProfiles) {
                projectProfileInto(sample.thetaDegree, sample.profile, cloudCopy);
            }
        } else if (!layer.originalPoints.isEmpty()) {
            cloudCopy = layer.originalPoints;
        }
    }
    if (cloudCopy.isEmpty() && !m_lastCloudRaw.isEmpty()) {
        cloudCopy = m_lastCloudRaw;
    }

    if (cloudCopy.isEmpty()) {
        emit logMessage("WRN", "No scanned data found for diameter calibration. Scan the cylinder first.");
        return;
    }

    // Dis ceper / tabla cevresi filtre: eksenden maxRadius disindaki noktalar
    // daire fit'ini bozar (fit, buyuk halkaya kacar). Sadece merkeze yakin
    // noktalar kalir.
    if (maxRadiusMm > 0.0f) {
        const float maxR2 = maxRadiusMm * maxRadiusMm;
        QVector<QVector3D> inner;
        inner.reserve(cloudCopy.size());
        for (const auto& p : cloudCopy) {
            if (p.x() * p.x() + p.y() * p.y() <= maxR2) {
                inner.push_back(p);
            }
        }
        const int removed = cloudCopy.size() - inner.size();
        cloudCopy = std::move(inner);
        if (removed > 0) {
            emit logMessage("SYS",
                QString("Diameter calibration: %2 points beyond %1 mm from the axis (rim) removed.")
                    .arg(maxRadiusMm, 0, 'f', 0).arg(removed));
        }
    }

    if (cloudCopy.isEmpty()) {
        emit logMessage("WRN", "Diameter calibration: no points left after the radius filter.");
        return;
    }

    emit processingStarted("Diameter Calibration (dOffset)");

    const float dOffsetNow = m_dOffset;
    const float lOffsetNow = m_lOffset;

    auto future = QtConcurrent::run(
        [cloudCopy, knownDiameterMm, zMinMm, zMaxMm]() -> CircleFitResult {
            return CalibrationEngine::fitCircleXY(cloudCopy, zMinMm, zMaxMm);
        });

    auto watcher = new QFutureWatcher<CircleFitResult>(this);
    connect(watcher, &QFutureWatcher<CircleFitResult>::finished, this,
            [this, watcher, knownDiameterMm, dOffsetNow, lOffsetNow]() {
        const CircleFitResult fit = watcher->result();
        watcher->deleteLater();
        emit processingFinished();

        if (!fit.success) {
            const QString msg = QString(
                "Diameter calibration failed: not enough/suitable points in the selected z-band "
                "(%1 points found in the band). Select the band from the cylinder's straight body.")
                    .arg(fit.totalPoints);
            emit logMessage("ERR", msg);
            emit diameterCalibrationFinished(false, msg, dOffsetNow);
            return;
        }

        const float rTrue = knownDiameterMm / 2.0f;

        // dOffset hatasi eps, olculen yaricapi yaklasik eps*sqrt(R^2-L^2)/R
        // kadar kaydirir (lazer hatti eksenden L kadar kacik oldugu icin).
        // Geometrik duzeltme faktoru ile eps'i geri cozuyoruz.
        float geomFactor = 1.0f;
        if (std::abs(lOffsetNow) < rTrue * 0.9f) {
            geomFactor = rTrue / std::sqrt(rTrue * rTrue - lOffsetNow * lOffsetNow);
        }
        const float eps = (fit.radiusMm - rTrue) * geomFactor;
        const float suggested = dOffsetNow - eps;

        const float centerDist = std::sqrt(fit.centerXMm * fit.centerXMm +
                                           fit.centerYMm * fit.centerYMm);

        QString report = QString(
            "Diameter Calibration Result\n\n"
            "[ Measurement ]\n"
            "• Measured diameter: %1 mm (known: %2 mm)\n"
            "• Cap farki: %3 mm\n"
            "• Fit RMS artigi: %4 mm\n"
            "• Merkez kaymasi (eksene gore): %5 mm (X:%6, Y:%7)\n"
            "• Points used: %8 / %9\n\n"
            "[ Oneri ]\n"
            "• dOffset error (eps): %10 mm\n"
            "• Current dOffset: %11 mm → Suggested: %12 mm")
                .arg(fit.radiusMm * 2.0f, 0, 'f', 3)
                .arg(knownDiameterMm, 0, 'f', 3)
                .arg((fit.radiusMm - rTrue) * 2.0f, 0, 'f', 3)
                .arg(fit.rmsResidualMm, 0, 'f', 3)
                .arg(centerDist, 0, 'f', 2)
                .arg(fit.centerXMm, 0, 'f', 2)
                .arg(fit.centerYMm, 0, 'f', 2)
                .arg(fit.usedPoints)
                .arg(fit.totalPoints)
                .arg(eps, 0, 'f', 3)
                .arg(dOffsetNow, 0, 'f', 2)
                .arg(suggested, 0, 'f', 2);

        // Guvenilirlik uyarilari
        if (fit.rmsResidualMm > 0.15f) {
            report += "\n\nWARNING: Fit RMS residual is high (>0.15 mm). First calibrate the lateral "
                      "offset, or select the band from a cleaner region.";
        }
        if (centerDist > 10.0f) {
            report += "\n\nWARNING: The cylinder is placed too far from the axis "
                      "(>10 mm). Place the part near the table center and scan again.";
        }

        emit logMessage("SYS", QString("Diameter calibration: measured=%1 mm, suggested dOffset=%2 mm")
                                   .arg(fit.radiusMm * 2.0f, 0, 'f', 3)
                                   .arg(suggested, 0, 'f', 2));
        emit diameterCalibrationFinished(true, report, suggested);
    });

    watcher->setFuture(future);
}
