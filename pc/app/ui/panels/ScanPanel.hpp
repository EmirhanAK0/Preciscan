#pragma once
#include <QWidget>
#include <QListWidget>
#include <QVector>
#include <QVector3D>
#include <QMap>

class ScanController;
class QTextEdit;
class QPushButton;
class QLabel;
class QSlider;
class QSpinBox;
class QComboBox;
class LayerItemWidget;
class QDoubleSpinBox;

/// Bir tarama katmaninin nokta bulutu ve meta verileri
struct ScanLayerData {
    QString            name;
    QVector<QVector3D> points;
    float              zOffsetMm = 0.0f;
};

class ScanPanel : public QWidget {
    Q_OBJECT
public:
    explicit ScanPanel(ScanController* controller, QWidget* parent = nullptr);

public slots:
    // ScanController sinyallerinden gelir
    void onScanStarted();
    void onScanStopped();
    void onMcuConnected(bool ok);
    void onLaserConnected(bool ok);

    /// Tarama durduğunda VisualizerWidget'tan noktalar buraya verilir
    void addLayer(const QVector<QVector3D>& points);

    // Dis kaynaklardan log itmek icin
    void appendLog(const QString& level, const QString& msg);

signals:
    /// Birlestirilmis nokta bulutu hazir — MainWindow 3D'de gosterir
    void mergedCloudReady(const QVector<QVector3D>& cloud);

private slots:
    void onStartClicked();
    void onStopClicked();
    void onMergeClicked();
    void onDeleteLayer(int layerId);
    void onLayerZOffsetChanged(int layerId, float mm);

private:
    void updateStartButtonState();
    void rebuildLayerListUI();

    ScanController* m_ctrl;

    // Kontrol durumu
    bool m_mcuReady   = false;
    bool m_laserReady = false;

    // Ayarlar
    QSlider*  m_speedSlider;
    QSpinBox* m_speedSpin;
    QSlider*  m_exposureSlider;
    QSpinBox* m_exposureSpin;

    // Kontrol
    QLabel*      m_scanStatusLabel;
    QPushButton* m_startBtn;
    QPushButton* m_stopBtn;

    // Katmanlar
    QListWidget* m_layerList;
    QPushButton* m_addLayerBtn;   // STL Sec
    QPushButton* m_mergeBtn;
    QPushButton* m_exportPlyBtn;  // PLY Disa Aktar
    QComboBox*   m_mergeMode;

    // Katman veri deposu
    QMap<int, ScanLayerData> m_layers;    ///< layerId -> veri
    int m_nextLayerId = 0;

    // Z-ekseni kontrol
    QDoubleSpinBox* m_zMoveSpin;
    QPushButton*    m_zMoveBtn;
    QPushButton*    m_zHomeBtn;
    QPushButton*    m_linHomeBtn;

    // Log
    QTextEdit* m_logView;
};
