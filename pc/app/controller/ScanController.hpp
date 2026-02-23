#pragma once
#include <QObject>
#include <atomic>
#include <QPointF>
#include <QVector3D>
#include <QVector>
#include <QString>
#include <QTimer>

class McuListener;
class LaserManager;
class SPSCRingBuffer;

namespace sim {
    class LaserSimWorker;
    struct Mesh;
}

class ScanController : public QObject {
    Q_OBJECT

public:
    ScanController(McuListener* mcu,
                   LaserManager* laser,
                   SPSCRingBuffer* ring,
                   QObject* parent = nullptr);
    ~ScanController();

    bool isMcuConnected()   const { return m_mcuConnected;   }
    bool isLaserConnected() const { return m_laserConnected; }
    bool isScanning()       const { return m_scanning;       }

public slots:
    void connectMcu();
    void disconnectMcu();
    void connectLaser();
    void connectLaserSim(const QString& stlPath);
    void disconnectLaser();

    void setDOffset(float mm);
    void setResolution(float deg);
    void setRps(float rps);

    void startScan();  
    void stopScan();
    void saveCurrentScan(const QString& path);

signals:
    void mcuConnectionChanged(bool connected);
    void laserConnectionChanged(bool connected);
    void isSimModeChanged(bool isSim);
    void scanStarted();
    void scanStopped();
    void simProgressUpdated(int percent);
    void requestClearVisualizer();
    void mcuPacketReceived(quint32 seq, float y_mm);
    void simProfileReceived(float theta_deg, const QVector<QPointF>& profile);
    void pointCloudReady(const QVector<QVector3D>& cloud);
    void meshLoaded(const QVector<QVector3D>& triangles);
    void logMessage(const QString& level, const QString& msg);

private slots:
    void consumeHardwarePackets(); // QTimer consumer for real laser

private:
    McuListener*    m_mcu;
    LaserManager*   m_laser;
    SPSCRingBuffer* m_ring;

    bool m_isSimMode{false};
    QString m_stlPath;
    sim::LaserSimWorker* m_simWorker{nullptr};
    float m_dOffset{66.0f};
    float m_resolution{1.0f};
    float m_rps{10.0f};
    QVector<QVector3D> m_lastCloud;

    // Hardware consumer
    QTimer* m_hwTimer{nullptr};
    float m_hwAngle{0.0f}; // current rotation angle for HW mode

    std::atomic<bool> m_mcuConnected{false};
    std::atomic<bool> m_laserConnected{false};
    std::atomic<bool> m_scanning{false};
};