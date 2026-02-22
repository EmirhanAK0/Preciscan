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

private:
    void setupToolBar();
    void setupCentralWidget();
    void setupStatusBar();
    void updateConnectButton(QPushButton* btn, bool connected,
                             const QString& baseName);

    AppStateMachine* m_stateMachine;
    ScanController*  m_scanController;

    // Toolbar butonlari
    QPushButton* m_mcuBtn;
    QPushButton* m_laserBtn;

    // StatusBar etiketleri
    QLabel* m_stateLabel;
    QLabel* m_mcuStatusLabel;
    QLabel* m_laserStatusLabel;

    QTabWidget* m_tabWidget;
};
