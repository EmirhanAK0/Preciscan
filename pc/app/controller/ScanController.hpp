#pragma once

#include <QObject>
#include <QPointF>
#include <QTimer>
#include <QString>
#include <QVector>
#include <QVector3D>
#include <queue>
#include <atomic>

#include "../../io/serial_trigger_reader.h"
#include "ScanProfileFrame.hpp"

/// Tarama tetik kaynağı
enum class ScanTriggerMode {
    TimeBased,      ///< Dahili zamanlayıcı (orijinal davranış)
    Encoder,        ///< MCU'dan gelen enkoder açısı tetikler
    ExternalTrigger ///< Harici dijital tetik sinyali (ayrı pin)
};

class McuListener;
class LaserManager;
class SPSCRingBuffer;

namespace sim {
class LaserSimWorker;
struct Mesh;
} // namespace sim

class ScanController : public QObject {
    Q_OBJECT

public:
    ScanController(McuListener* mcu,
                   LaserManager* laser,
                   SPSCRingBuffer* ring,
                   QObject* parent = nullptr);
    ~ScanController();

    bool isMcuConnected() const { return m_mcuConnected; }
    bool isLaserConnected() const { return m_laserConnected; }
    bool isScanning() const { return m_scanning; }

    float dOffset() const { return m_dOffset; }
    float lateralOffset() const { return m_lOffset; }
    float resolution() const { return m_resolution; }
    float rps() const { return m_rps; }

    const QVector<QVector3D>& getLastCloud() const { return m_lastCloud; }
    void setLastCloud(const QVector<QVector3D>& cloud);

    int laserProfileRate() const { return m_laserProfileRate; }
    int laserShutterUs() const { return m_laserShutterUs; }
    bool laserAutoShutter() const { return m_laserAutoShutter; }
    QString laserMeasuringField() const { return m_laserMeasuringField; }
    int laserPointsPerProfile() const { return m_laserPointsPerProfile; }

    ScanTriggerMode triggerMode() const { return m_triggerMode; }

public slots:
    void connectMcu();
    void disconnectMcu();
    void connectLaser();
    void connectLaserSim(const QString& stlPath);
    void disconnectLaser();

    void setDOffset(float mm);
    void setLateralOffset(float mm);
    void setResolution(float deg);
    void setRps(float rps);

    void setLaserProfileRate(int hz);
    void setLaserShutterUs(int us);
    void setLaserAutoShutter(bool enabled);
    void setLaserMeasuringField(const QString& field);
    void setLaserPointsPerProfile(int points);
    void setTriggerMode(ScanTriggerMode mode);
    void setSerialPort(const QString& portName);

    void onEncoderTrigger(float angleDeg);

    void connectMcuSerial();
    void disconnectMcuSerial();

    void startScan();
    void stopScan();
    void saveCurrentScan(const QString& path);

    bool sendSerialCommand(const QString& cmd);
    void sendZMove(float mm);
    void sendZHome();
    void sendLinHome();

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
    void profileFrameReceived(const ScanProfileFrame& frame);
    void pointCloudReady(const QVector<QVector3D>& cloud);
    void meshLoaded(const QVector<QVector3D>& triangles);
    void logMessage(const QString& level, const QString& msg);
    void triggerModeChanged(ScanTriggerMode mode);
    void historySizeChanged(int size);

public slots:
    void applyFilterCylindrical(float radiusMm, float minZ, float maxZ);
    void applyFilterStatistical(int meanK, float stdDevThresh);
    void applyFilterRadius(float radiusMm, int minNeighbors);
    void applyFilterVoxelGrid(float leafSizeMm);

    void applyManualDeletion(const QVector<int>& indicesToRemove);
    void undoLastFilter();
    void resetCloud();

private slots:
    void consumeHardwarePackets();

private:
    void rebuildSimWorkerIfPossible();
    bool validateLaserTiming(QString* errorMsg = nullptr) const;
    void publishProfileFrame(float thetaDeg, const QVector<QPointF>& profile);

private:
    McuListener* m_mcu = nullptr;
    LaserManager* m_laser = nullptr;
    SPSCRingBuffer* m_ring = nullptr;

    bool m_isSimMode{false};
    QString m_stlPath;
    sim::LaserSimWorker* m_simWorker{nullptr};

    float m_dOffset{78.5f};
    float m_lOffset{2.0f};
    float m_resolution{1.0f};
    float m_rps{10.0f};

    int m_laserProfileRate{100};
    int m_laserShutterUs{100};
    bool m_laserAutoShutter{true};
    QString m_laserMeasuringField{"large"};
    int m_laserPointsPerProfile{1280};

    QVector<QVector3D> m_originalCloud;
    QVector<QVector3D> m_lastCloud;
    QVector<QVector<QVector3D>> m_cloudHistory;

    QTimer* m_hwTimer{nullptr};
    float m_hwAngle{0.0f};
    std::queue<float> m_encoderAngles;

    ScanTriggerMode m_triggerMode{ScanTriggerMode::Encoder};

    QString m_serialPort{"COM3"};
    SerialTriggerReader* m_serialReader{nullptr};

    std::atomic<bool> m_mcuConnected{false};
    std::atomic<bool> m_laserConnected{false};
    std::atomic<bool> m_scanning{false};
};