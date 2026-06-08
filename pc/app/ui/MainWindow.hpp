#pragma once
#include <QMainWindow>
#include "state/AppState.hpp"

class AppStateMachine;
class ScanController;
class QTabWidget;
class QLabel;
class QPushButton;
class McuListener;
class LaserManager;
class SPSCRingBuffer;
class ConnectingOverlay;
class QComboBox;
class VisualizerWidget;
class ScanPanel;
class QProgressDialog;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(McuListener* mcu,
                        LaserManager* laser,
                        SPSCRingBuffer* ring,
                        QWidget* parent = nullptr);

public slots:
    void onStateChanged(AppState newState);
    void onMcuConnectionChanged(bool connected);
    void onLaserConnectionChanged(bool connected);
    void onProcessingStarted(const QString& taskName);
    void onProcessingFinished();

private:
    void setupToolBar();
    void setupCentralWidget();
    void setupStatusBar();
    void updateConnectButton(QPushButton* btn, bool connected,
                             const QString& baseName);

    AppStateMachine* m_stateMachine;
    ScanController*  m_scanController;
    QProgressDialog* m_progressDialog;

    // Toolbar butonlari
    QPushButton* m_mcuBtn;
    QPushButton* m_laserBtn;
    QComboBox* m_comPortCombo;

    // StatusBar etiketleri
    QLabel* m_stateLabel;
    QLabel* m_mcuStatusLabel;
    QLabel* m_laserStatusLabel;

    QTabWidget*        m_tabWidget;
    ConnectingOverlay* m_connectingOverlay = nullptr;
    VisualizerWidget*  m_viz               = nullptr;
    ScanPanel*         m_scanPanel         = nullptr;
};
