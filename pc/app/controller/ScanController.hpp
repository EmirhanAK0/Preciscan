#pragma once

#include <QObject>
#include <QPointF>
#include <QTimer>
#include <QString>
#include <QVector>
#include <QVector3D>
#include <queue>
#include <deque>
#include <atomic>
#include <QFutureWatcher>
#include <QFuture>
#include <QtConcurrent>
#include <QMap>

#include "../../io/serial_trigger_reader.h"
#include "../utils/AutoCalibrator.h"
#include "ScanProfileFrame.hpp"

/// Bir tarama katmaninin nokta bulutu ve meta verileri
struct ScanLayerData {
    QString            name;
    QVector<QVector3D> points;
    QVector<QVector3D> originalPoints;
    QVector<QVector<QVector3D>> history;
    /// Projeksiyon oncesi ham (aci, profil) verisi. Offset/kalibrasyon
    /// degisince katman bu veriden yeniden projekte edilir.
    QVector<RawProfileSample> rawProfiles;
    float              zOffsetMm = 0.0f;
};

/// Tarama tetik kaynağı
enum class ScanTriggerMode {
    TimeBased,      ///< Dahili zamanlayıcı (orijinal davranış)
    Encoder,        ///< MCU'dan gelen enkoder açısı tetikler
    ExternalTrigger ///< Harici dijital tetik sinyali (ayrı pin)
};

/// Tarama tetik kaynağı (Donanımsal açı bilgisi nereden geliyor)
enum class TriggerSource {
    AS5600 = 0,     ///< Manyetik enkoderden gelen açı (orijinal davranış)
    StepAngle = 1   ///< Motor adımları üzerinden hesaplanan açı
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
    float zBaseOffset() const { return m_zBaseOffset; }
    float as5600Resolution() const { return m_as5600Resolution; }
    float stepResolution() const { return m_stepResolution; }
    float secPerRev() const { return m_secPerRev; }

    const QVector<QVector3D>& getLastCloud() const;
    void setLastCloud(const QVector<QVector3D>& cloud);

    const QMap<int, ScanLayerData>& getLayers() const { return m_layers; }
    int getActiveLayerId() const { return m_activeLayerId; }

    int laserProfileRate() const { return m_laserProfileRate; }
    int laserShutterUs() const { return m_laserShutterUs; }
    bool laserAutoShutter() const { return m_laserAutoShutter; }
    QString laserMeasuringField() const { return m_laserMeasuringField; }
    int laserPointsPerProfile() const { return m_laserPointsPerProfile; }

    ScanTriggerMode triggerMode() const { return m_triggerMode; }
    TriggerSource triggerSource() const { return m_triggerSource; }

public slots:
    void connectMcu();
    void disconnectMcu();
    void connectLaser();
    void connectLaserSim(const QString& stlPath);
    void disconnectLaser();

    void setDOffset(float mm);
    void setLateralOffset(float mm);
    void setZBaseOffset(float mm);

    /// Aktif katmanin taban duzlemini (tabla yuzeyini) bulup z=0'a oturtur:
    /// m_zBaseOffset otomatik duzeltilir ve katman yeniden projekte edilir.
    void autoZeroBase(float maxRadiusMm = 45.0f);
    void setAs5600Resolution(float deg);
    void setStepResolution(float deg);
    void setSecPerRev(float sec);

    void setLaserProfileRate(int hz);
    void setLaserShutterUs(int us);
    void setLaserAutoShutter(bool enabled);
    void setLaserMeasuringField(const QString& field);
    void setLaserPointsPerProfile(int points);
    void setTriggerMode(ScanTriggerMode mode);
    void setTriggerSource(TriggerSource src);
    void setSerialPort(const QString& portName);

    void onEncoderTrigger(float angleDeg);

    void connectMcuSerial();
    void disconnectMcuSerial();

    void startScan(int direction = 0); // 0: CW, 1: CCW
    void stopScan();
    void saveCurrentScan(const QString& path);

    bool sendSerialCommand(const QString& cmd);
    void sendZMove(float mm);
    void sendZHome();
    void sendLinHome();
    
    void mergeWithICPAsync(const QVector<QVector3D>& target, const QVector<QVector3D>& source, int icpMode);
    void mergeSelectedLayers(const QVector<int>& layerIds, const QString& mode);
    void generateMeshAsync();
    void clearMesh();

    void setActiveLayer(int id);
    void deleteLayer(int id);
    void setLayerName(int id, const QString& name);
    void setLayerZOffset(int id, float mm);
    void addNewLayer(const QVector<QVector3D>& points, const QString& name = QString());

    AutoCalibrator* getCalibrator() const { return m_calibrator; }

signals:
    void mcuConnectionChanged(bool connected);
    void laserConnectionChanged(bool connected);
    void isSimModeChanged(bool isSim);
    void scanStarted();
    void scanStopped();
    void simProgressUpdated(int percent);
    void icpMergeFinished(const QVector<QVector3D>& mergedCloud);
    void processingStarted(const QString& taskName);
    void processingFinished();
    void requestClearVisualizer();
    void mcuPacketReceived(quint32 seq, float y_mm);
    void simProfileReceived(float theta_deg, const QVector<QPointF>& profile);
    void profileFrameReceived(const ScanProfileFrame& frame);
    void pointCloudReady(const QVector<QVector3D>& cloud);
    void meshLoaded(const QVector<QVector3D>& triangles);
    void logMessage(const QString& level, const QString& msg);
    void triggerModeChanged(ScanTriggerMode mode);
    void triggerSourceChanged(TriggerSource src);
    void historySizeChanged(int size);
    void activeLayerChanged(int id);
    void layersUpdated();
    void autoCalibrationFinished(float bestOffset, double score);
    void calibration3DFinished(bool success, QString report);
    void diameterCalibrationFinished(bool success, QString report, float suggestedDOffset);
    void zBaseOffsetChanged(float mm);

public slots:
    void updateActiveCalibration(QString filePath);
    void disableCalibration();
    void applyFilterCylindrical(float radiusMm, float minZ, float maxZ);
    void applyFilterStatistical(int meanK, float stdDevThresh);
    void applyFilterRadius(float radiusMm, int minNeighbors);
    void applyFilterVoxelGrid(float leafSizeMm);
    void clearHistory();

    void applyManualDeletion(const QVector<int>& indicesToRemove);
    void undoLastFilter();
    void resetCloud();

    // Manuel Hizalama
    void beginManualAlignment();
    void updateManualAlignment(float tx, float ty, float tz, float rx, float ry, float rz);
    void commitManualAlignment();
    void cancelManualAlignment();

    // Otomatik Kalibrasyon
    void autoCalibrateOffsetAsync();
    void start3DCalibrationAsync(int method = 0);

    /// Bilinen capli silindir bandindan dOffset kalibrasyonu:
    /// [zMin, zMax] araligindaki noktalara daire fit edip olculen yaricapi
    /// bilinen yaricapla karsilastirir; onerilen dOffset'i sinyalle dondurur.
    /// maxRadiusMm: eksenden bu yaricapin disindaki noktalar (tabla cevresi,
    /// dis ceper vb.) fit'e alinmaz.
    void calibrateDiameterAsync(float knownDiameterMm, float zMinMm, float zMaxMm,
                                float maxRadiusMm = 40.0f);

private slots:
    void consumeHardwarePackets();
    void onMeshFinished();
    void onFilterFinished();
    void onIcpFinished();


private:
    void rebuildSimWorkerIfPossible();
    bool validateLaserTiming(QString* errorMsg = nullptr) const;
    void publishProfileFrame(float thetaDeg, const QVector<QPointF>& profile);

    /// Tek profili mevcut dOffset/lateral offset ile 3D'ye projekte eder
    /// (kalibrasyonsuz ham noktalar 'out'a eklenir).
    void projectProfileInto(float thetaDeg, const QVector<QPointF>& profile,
                            QVector<QVector3D>& out) const;

    /// Aktif katmani ham (aci, profil) verisinden yeniden projekte eder.
    /// Offset degisikliklerinin yeniden tarama gerektirmemesini saglar.
    void reprojectActiveLayerFromRaw();

private:
    McuListener* m_mcu = nullptr;
    LaserManager* m_laser = nullptr;
    SPSCRingBuffer* m_ring = nullptr;

    bool m_isSimMode{false};
    QString m_stlPath;
    sim::LaserSimWorker* m_simWorker{nullptr};

    float m_dOffset{78.5f};
    float m_lOffset{2.0f};
    /// Profil x ekseni -> dunya Z donusumu tabani: z = p.x() - m_zBaseOffset.
    /// Tabla yuzeyinin z=0'a oturmasi icin kullanilir (Oto. Sifirla ile bulunur).
    float m_zBaseOffset{3.5f};
    float m_as5600Resolution{1.0f};
    float m_stepResolution{1.0f};
    float m_secPerRev{30.0f};

    int m_laserProfileRate{100};
    int m_laserShutterUs{100};
    bool m_laserAutoShutter{true};
    QString m_laserMeasuringField{"large"};
    int m_laserPointsPerProfile{1280};

    QVector<QVector3D> m_originalCloud;
    QVector<QVector3D> m_lastCloud;
    QVector<QVector<QVector3D>> m_cloudHistory;
    QVector<QVector3D> m_manualAlignBaseCloud;

    QTimer* m_hwTimer{nullptr};
    float m_hwAngle{0.0f};

    /// Bekleyen tetik olayi (MCU'dan gelen seq + aci).
    struct PendingTrigger {
        quint32 seq;
        float angleDeg;
    };

    /// FIFO eslestirme kuyruklari: her donanim tetigi tam olarak
    /// 1 aci paketi + 1 lazer profili uretir; sirayla birebir eslesirler.
    std::deque<PendingTrigger> m_pendingTriggers;
    std::deque<QVector<QPointF>> m_pendingProfiles;

    /// Devam eden taramanin ham (aci, profil) kayitlari.
    QVector<RawProfileSample> m_rawScanData;

    // Tani sayaclari (tarama sonunda raporlanir)
    quint32 m_statAngles{0};          ///< Alinan tetik/aci paketi sayisi
    quint32 m_statProfiles{0};        ///< Lazerden alinan profil sayisi
    quint32 m_statEmptyProfiles{0};   ///< Bos/donusturulemeyen profil sayisi
    quint32 m_statPaired{0};          ///< Eslestirilen (aci, profil) cifti
    quint32 m_statSeqGaps{0};         ///< Seri portta kaybolan tetik paketi
    quint32 m_statDroppedProfiles{0}; ///< Tasma nedeniyle atilan profil
    quint32 m_lastSeq{0};
    bool m_hasLastSeq{false};

    ScanTriggerMode m_triggerMode{ScanTriggerMode::Encoder};
    TriggerSource m_triggerSource{TriggerSource::AS5600};

    QString m_serialPort{"COM3"};
    SerialTriggerReader* m_serialReader{nullptr};

    bool m_hasStartAngle{false};
    float m_startAngle{0.0f};

    std::atomic<bool> m_mcuConnected{false};
    std::atomic<bool> m_laserConnected{false};
    std::atomic<bool> m_scanning{false};

    QFutureWatcher<QVector<QVector3D>> m_watcher;
    bool m_meshGenerating = false;
    
    QFutureWatcher<QVector<QVector3D>> m_icpWatcher;
    bool m_icpRunning = false;
    
    QString m_currentTaskName;
    
    QVector<QVector3D> m_lastCloudRaw;
    
    QMap<int, ScanLayerData> m_layers;
    int m_nextLayerId{0};
    int m_activeLayerId{-1};

    AutoCalibrator* m_calibrator{nullptr};

    void reapplyCurrentCalibration();
};