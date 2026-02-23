#include "ScanController.hpp"
#include "../../net/mcu_listener.h"
#include "../../hardware/laser_manager.h"
#include "../../net/spsc_ring_buffer.h"
#include "../../sim/mesh_slicer.h"
#include "../../sim/laser_sim_worker.h"
#include "../../io/stl_loader.h"
#include "../../io/ply_writer.h"
#include "../../core/packet.h"
#include <QDebug>
#include <QFileInfo>
#include <cstring>

ScanController::ScanController(McuListener* mcu, LaserManager* laser, 
                               SPSCRingBuffer* ring, QObject* parent)
    : QObject(parent), m_mcu(mcu), m_laser(laser), m_ring(ring) 
{
    // Hardware consumer timer (non-blocking, UI-thread safe)
    m_hwTimer = new QTimer(this);
    m_hwTimer->setInterval(0); // Process as fast as possible (event loop idle)
    connect(m_hwTimer, &QTimer::timeout, this, &ScanController::consumeHardwarePackets);
}

ScanController::~ScanController() {
    if (m_simWorker) {
        m_simWorker->stop();
        m_simWorker->wait();
        delete m_simWorker;
    }
}

void ScanController::connectMcu() { emit mcuConnectionChanged(true); }
void ScanController::disconnectMcu() { emit mcuConnectionChanged(false); }
void ScanController::connectLaser() {
    if (!m_laser) {
        emit logMessage("ERR", "LaserManager mevcut degil!");
        return;
    }
    if (!m_laser->init()) {
        emit logMessage("ERR", "Lazer SDK yuklenemedi (LLT.dll)! Lazer bagli mi?");
        return;
    }
    if (!m_laser->connect()) {
        emit logMessage("ERR", "Lazere baglanilamadi! Ethernet/USB kontrolü yapiniz.");
        return;
    }
    m_laser->startAcquisition();
    m_isSimMode = false;
    emit laserConnectionChanged(true);
    emit logMessage("OK", "Lazer baglandi ve veri akisi baslatildi.");
}
void ScanController::connectLaserSim(const QString& stlPath) {
    if (m_scanning) return;
    
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
    
    QVector<QVector3D> meshPoints;
    meshPoints.reserve(static_cast<int>(mesh.triangles.size() * 3));
    for (const auto& tri : mesh.triangles) {
        for (int i = 0; i < 3; ++i) {
            meshPoints.push_back(QVector3D(tri.v[i].x, tri.v[i].y, tri.v[i].z));
        }
    }
    emit meshLoaded(meshPoints);
    emit laserConnectionChanged(true);
    emit logMessage("OK", QString("Lazer simülasyonu hazır: %1").arg(QFileInfo(stlPath).fileName()));
    
    sim::SliceParams p;
    p.D_offset_mm = m_dOffset;
    p.deg_per_step = m_resolution;
    p.rps = m_rps;

    if (m_simWorker) {
        m_simWorker->stop();
        m_simWorker->wait();
        delete m_simWorker;
    }

    m_simWorker = new sim::LaserSimWorker(std::move(mesh), p, this);
    
    connect(m_simWorker, &sim::LaserSimWorker::profileReady, this, [this](float theta, const QVector<QPointF>& sensorData){
        QVector<QPointF> reconstructed;
        reconstructed.reserve(sensorData.size());
        for (const auto& p : sensorData) {
            float heightZ = p.x();
            float distD = p.y();
            float radiusR = m_dOffset - distD;
            if (radiusR > 0.05f) {
                reconstructed.push_back(QPointF(radiusR, heightZ));
            }
        }
        if (!reconstructed.isEmpty()) {
            emit simProfileReceived(theta, reconstructed);
        }
    });

    connect(m_simWorker, &sim::LaserSimWorker::progressUpdated, this, &ScanController::simProgressUpdated);
    connect(m_simWorker, &sim::LaserSimWorker::scanComplete, this, [this](const QVector<QVector3D>& cloud){
        m_lastCloud = cloud;
        emit pointCloudReady(cloud);
        stopScan();
    });
}
void ScanController::disconnectLaser() {
    if (m_laser) m_laser->stopAcquisition();
    emit laserConnectionChanged(false);
    emit logMessage("SYS", "Lazer baglantisi kesildi.");
}

void ScanController::startScan() {
    if (m_scanning) return;
    emit requestClearVisualizer();
    m_scanning = true;
    m_lastCloud.clear();
    m_hwAngle = 0.0f;
    if (m_isSimMode && m_simWorker) {
        m_simWorker->start();
    } else if (m_ring) {
        // Real hardware: start the QTimer consumer
        m_hwTimer->start();
    }
    emit scanStarted();
}

void ScanController::stopScan() {
    if (!m_scanning) return;
    m_scanning = false;
    if (m_isSimMode && m_simWorker) m_simWorker->stop();
    m_hwTimer->stop();
    emit scanStopped();
}

// -------------------------------------------------------------------
// Real hardware packet consumer — runs on UI event loop (non-blocking)
// -------------------------------------------------------------------
// LLT SDK ham paketi:
//   Her profil, birden cok uint16_t Y-mesafe değeri içerir.
//   Her uint16_t = mesafe değeri (ham). 
//   Her örnek, belirli bir piksel/irtifa pozisyonunu temsil eder.
//   Piksel başı Z aralığı sensör modeline göre değişir; 
//   bu basit versiyonda lineer dağılım varsayıyoruz (0..22 mm).
void ScanController::consumeHardwarePackets() {
    if (!m_ring || !m_scanning) return;

    static constexpr int MAX_PER_TICK = 8; // Max 8 paket per event loop tick
    Packet pkt;
    int processed = 0;

    while (processed < MAX_PER_TICK && m_ring->try_pop(pkt)) {
        ++processed;
        const size_t sz = pkt.data.size();
        if (sz < 4) continue;

        // LLT SDK: profil verisi uint16_t dizisi olarak gelir
        // Her uint16_t = tek bir nokta. Birim: 1/100 mm (0.01 mm)
        const uint16_t* samples = reinterpret_cast<const uint16_t*>(pkt.data.data());
        const int sampleCount = static_cast<int>(sz / sizeof(uint16_t));
        if (sampleCount < 1) continue;

        QVector<QPointF> profile;
        profile.reserve(sampleCount);

        for (int i = 0; i < sampleCount; ++i) {
            float dist_mm = samples[i] * 0.01f;        // 0.01 mm/LSB
            float height_mm = (float)i * 22.0f / sampleCount; // lineer 0-22 mm
            
            if (dist_mm < 0.5f || dist_mm > m_dOffset) continue; // Geçersiz
            profile.push_back(QPointF(height_mm, dist_mm));
        }

        if (!profile.isEmpty()) {
            emit simProfileReceived(m_hwAngle, profile);
            // Bir sonraki açıya ilerle
            m_hwAngle += m_resolution;
            if (m_hwAngle >= 360.0f) m_hwAngle = 0.0f;
        }
    }
}

void ScanController::saveCurrentScan(const QString& path) {
    if (m_lastCloud.isEmpty()) return;
    io::writePLY(path, m_lastCloud);
    emit logMessage("OK", QString("Kaydedildi: %1").arg(path));
}

void ScanController::setDOffset(float val) { m_dOffset = val; }
void ScanController::setResolution(float val) { m_resolution = val; }
void ScanController::setRps(float val) { m_rps = val; }