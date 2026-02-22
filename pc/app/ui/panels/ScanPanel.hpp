#pragma once
#include <QWidget>
#include <QListWidget>

class ScanController;
class QTextEdit;
class QPushButton;
class QLabel;
class QSlider;
class QSpinBox;
class QComboBox;

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

    // Dis kaynaklardan log itmek icin
    void appendLog(const QString& level, const QString& msg);

private slots:
    void onStartClicked();
    void onStopClicked();
    void onMergeClicked();
    void onAddLayerDemo();   // Simule: yeni katman ekleme

private:
    void updateStartButtonState();

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
    QPushButton* m_addLayerBtn;   // Demo amacliymiş
    QPushButton* m_mergeBtn;
    QComboBox*   m_mergeMode;

    // Log
    QTextEdit* m_logView;
};
