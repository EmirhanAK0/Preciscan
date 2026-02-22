#include "ScanController.hpp"
#include "../../net/mcu_listener.h"
#include "../../hardware/laser_manager.h"
#include "../../net/spsc_ring_buffer.h"
#include "../../sim/mesh_slicer.h"
#include "../../sim/laser_sim_worker.h"
#include "../../io/stl_loader.h"
#include "../../io/ply_writer.h"
#include <QDebug>
#include <QFileInfo>

ScanController::ScanController(McuListener* mcu, LaserManager* laser, 
                               SPSCRingBuffer* ring, QObject* parent)
    : QObject(parent), m_mcu(mcu), m_laser(laser), m_ring(ring) 
{
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
void ScanController::connectLaser() { emit laserConnectionChanged(true); }
void ScanController::disconnectLaser() { emit laserConnectionChanged(false); }

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
    
    connect(m_simWorker, &sim::LaserSimWorker::profileReady, this, &ScanController::simProfileReceived);
    connect(m_simWorker, &sim::LaserSimWorker::progressUpdated, this, &ScanController::simProgressUpdated);
    connect(m_simWorker, &sim::LaserSimWorker::scanComplete, this, [this](const QVector<QVector3D>& cloud){
        m_lastCloud = cloud;
        emit pointCloudReady(cloud);
        stopScan();
    });
}

void ScanController::startScan() {
    if (m_scanning) return;
    emit requestClearVisualizer();
    m_scanning = true;
    m_lastCloud.clear();
    if (m_isSimMode && m_simWorker) m_simWorker->start();
    emit scanStarted();
}

void ScanController::stopScan() {
    if (!m_scanning) return;
    m_scanning = false;
    if (m_isSimMode && m_simWorker) m_simWorker->stop();
    emit scanStopped();
}

void ScanController::saveCurrentScan(const QString& path) {
    if (m_lastCloud.isEmpty()) return;
    io::writePLY(path, m_lastCloud);
    emit logMessage("OK", QString("Kaydedildi: %1").arg(path));
}

void ScanController::setDOffset(float val) { m_dOffset = val; }
void ScanController::setResolution(float val) { m_resolution = val; }
void ScanController::setRps(float val) { m_rps = val; }