#include "MainWindow.hpp"

#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QStatusBar>
#include <QTabWidget>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>
#include <QProgressDialog>

#include "../../io/ply_writer.h"

#include "../controller/ScanController.hpp"
#include "panels/DiagnosticsPanel.hpp"
#include "panels/ScanPanel.hpp"
#include "panels/SetupPanel.hpp"
#include "panels/ProcessPanel.hpp"
#include "state/AppStateMachine.hpp"
#include "theme/Theme.hpp"
#include "widgets/ConnectingOverlay.hpp"
#include "widgets/ProfileWidget.hpp"
#include "widgets/VisualizerWidget.hpp"

static QLabel* dotLabel(const QString& txt, const QString& color, QWidget* p)
{
    auto* l = new QLabel(txt, p);
    l->setStyleSheet(QString("color:%1; font-size:11px; padding:0 6px;").arg(color));
    return l;
}

MainWindow::MainWindow(McuListener* mcu, LaserManager* laser, SPSCRingBuffer* ring, QWidget* parent)
    : QMainWindow(parent)
{
    m_stateMachine   = new AppStateMachine(this);
    m_scanController = new ScanController(mcu, laser, ring, this);
    m_progressDialog = nullptr;

    setWindowTitle("Preciscan");
    resize(1200, 750);

    setupToolBar();
    setupCentralWidget();
    setupStatusBar();

    // Baglanti overlay — butona basilinca goster, sonuca gore success/error
    m_connectingOverlay = new ConnectingOverlay(this);

    connect(m_stateMachine, &AppStateMachine::stateChanged, this, &MainWindow::onStateChanged);
    connect(m_scanController, &ScanController::scanStarted, this,
            [this] { m_stateMachine->setState(AppState::Scanning); });
    connect(m_scanController, &ScanController::processingStarted, this, &MainWindow::onProcessingStarted);
    connect(m_scanController, &ScanController::processingFinished, this, &MainWindow::onProcessingFinished);
    connect(m_scanController, &ScanController::scanStopped, this,
            [this] {
                m_stateMachine->setState(AppState::Idle);
                if (m_viz && m_scanPanel && m_viz->pointCount() > 0) {
                    m_scanPanel->addLayer(m_viz->getPoints());
                }
            });

    connect(m_scanController, &ScanController::mcuConnectionChanged, this,
            &MainWindow::onMcuConnectionChanged);
    connect(m_scanController, &ScanController::laserConnectionChanged, this,
            &MainWindow::onLaserConnectionChanged);
    connect(m_scanController, &ScanController::laserConnectStarted, this,
            &MainWindow::onLaserConnectStarted);
    connect(m_scanController, &ScanController::laserConnectFailed, this,
            &MainWindow::onLaserConnectFailed);

    onStateChanged(m_stateMachine->currentState());
    onMcuConnectionChanged(false);
    onLaserConnectionChanged(false);
}

void MainWindow::setupToolBar()
{
    auto* tb = addToolBar("Hardware");
    tb->setMovable(false);

    m_comPortCombo = new QComboBox(this);
    for (int i = 1; i <= 15; ++i) {
        m_comPortCombo->addItem(QString("COM%1").arg(i));
    }
    // Varsayilan COM portunu ayarla
    m_comPortCombo->setCurrentText("COM3");
    tb->addWidget(m_comPortCombo);

    m_mcuBtn = new QPushButton("MCU  Connect", this);
    m_mcuBtn->setObjectName("connectBtn");
    m_mcuBtn->setCheckable(true);
    connect(m_mcuBtn, &QPushButton::clicked, this,
            [this](bool checked)
            {
                if (checked) {
                    m_scanController->setSerialPort(m_comPortCombo->currentText());
                    m_scanController->connectMcuSerial();
                } else {
                    m_scanController->disconnectMcuSerial();
                }
            });
    tb->addWidget(m_mcuBtn);

    auto* spacerL = new QWidget;
    spacerL->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tb->addWidget(spacerL);

    auto* title = new QLabel("PRECISCAN", this);
    title->setStyleSheet("color:#eee; font-size:14px; font-weight:bold; letter-spacing:3px;");
    title->setAlignment(Qt::AlignCenter);
    tb->addWidget(title);

    auto* spacerR = new QWidget;
    spacerR->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tb->addWidget(spacerR);

    m_laserBtn = new QPushButton("Laser  Connect", this);
    m_laserBtn->setObjectName("connectBtn");
    m_laserBtn->setCheckable(true);
    connect(m_laserBtn, &QPushButton::clicked, this,
            [this](bool checked)
            {
                if (checked)
                {
                    m_connectingOverlay->showConnecting();
                    m_scanController->connectLaser();
                }
                else
                {
                    m_scanController->disconnectLaser();
                }
            });
    tb->addWidget(m_laserBtn);
}

void MainWindow::setupCentralWidget()
{
    auto* central = new QWidget(this);
    setCentralWidget(central);
    auto* hLayout = new QHBoxLayout(central);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);

    m_viz               = new VisualizerWidget(this);
    auto* viz           = m_viz;
    auto* profileWidget = new ProfileWidget(this);
    profileWidget->setMinimumHeight(160);
    profileWidget->setMaximumHeight(280);

    // 3D viz + 2D profil: dikey splitter
    auto* leftSplitter = new QSplitter(Qt::Vertical, this);
    leftSplitter->addWidget(viz);
    leftSplitter->addWidget(profileWidget);
    leftSplitter->setStretchFactor(0, 3);
    leftSplitter->setStretchFactor(1, 1);
    leftSplitter->setHandleWidth(3);
    leftSplitter->setStyleSheet("QSplitter::handle { background: #1a1a1a; }");

    m_tabWidget = new QTabWidget(this);
    // Sabit genislik yerine esnek: uzun butonlar tasmaz, splitter ile ayarlanabilir.
    m_tabWidget->setMinimumWidth(340);
    m_tabWidget->setMaximumWidth(460);

    m_scanPanel  = new ScanPanel(m_scanController, this);
    auto* scanPanel  = m_scanPanel;
    auto* setupPanel = new SetupPanel(m_scanController, this);
    auto* diagPanel  = new DiagnosticsPanel(this);
    auto* processPanel = new ProcessPanel(m_scanController, m_viz, this);

    m_tabWidget->addTab(scanPanel, "Scan");
    m_tabWidget->addTab(setupPanel, "Setup");
    m_tabWidget->addTab(diagPanel, "Diagnostics");
    m_tabWidget->addTab(processPanel, "Data Processing");

    // Sinyal baglantilari
    connect(m_scanController, &ScanController::mcuConnectionChanged, scanPanel,
            &ScanPanel::onMcuConnected);
    connect(m_scanController, &ScanController::laserConnectionChanged, scanPanel,
            &ScanPanel::onLaserConnected);

    // Simulation signals -> 3D Visualizer
    connect(m_scanController, &ScanController::simProfileReceived, viz,
            [this, viz](float theta, const QVector<QPointF>& profile)
            { viz->addProfile(theta, profile, m_scanController->dOffset(), m_scanController->zBaseOffset(), m_scanController->lateralOffset()); });
    // Simulation signals -> 2D Profile Widget
    connect(m_scanController, &ScanController::simProfileReceived, profileWidget,
            &ProfileWidget::updateProfile);
    // Temizleme sinyali
    connect(m_scanController, &ScanController::requestClearVisualizer, profileWidget,
            &ProfileWidget::clear);

    // Mesh ve Temizleme baglantilari
    connect(m_scanController, &ScanController::meshLoaded, viz, &VisualizerWidget::setMesh);
    connect(m_scanController, &ScanController::requestClearVisualizer, viz,
            &VisualizerWidget::clearPoints);

    connect(m_scanController, &ScanController::pointCloudReady, this,
            [this, viz](const QVector<QVector3D>& cloud)
            {
                viz->clearPoints();
                viz->addPoints(cloud);
            });

    // MCU log seyreltme
    static int mcuCounter = 0;
    connect(m_scanController, &ScanController::mcuPacketReceived, this,
            [scanPanel](quint32 seq, float y)
            {
                if (++mcuCounter >= 100)
                {
                    mcuCounter = 0;
                    scanPanel->appendLog(
                        "MCU",
                        QString("Receiving signal... Trigger:%1 Angle:%2 deg").arg(seq).arg(y, 0, 'f', 2));
                }
            });



    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(leftSplitter);
    splitter->addWidget(m_tabWidget);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);
    splitter->setHandleWidth(2);
    splitter->setStyleSheet("QSplitter::handle { background: #222; }");

    hLayout->addWidget(splitter);
}

void MainWindow::setupStatusBar()
{
    auto* sb = statusBar();

    m_stateLabel = dotLabel("State: Idle", "#888", this);
    sb->addWidget(m_stateLabel);

    sb->addWidget(dotLabel("|", "#333", this));
    m_mcuStatusLabel = dotLabel("MCU: Disconnected", "#e74c3c", this);
    sb->addWidget(m_mcuStatusLabel);

    sb->addWidget(dotLabel("|", "#333", this));
    m_laserStatusLabel = dotLabel("Laser: Disconnected", "#e74c3c", this);
    sb->addWidget(m_laserStatusLabel);

    sb->addPermanentWidget(dotLabel("Preciscan v2.1", "#333", this));
}

void MainWindow::onStateChanged(AppState s)
{
    static const char* names[] = {"Idle", "Ready", "Scanning", "Paused", "Completed", "Fault"};
    int idx                    = static_cast<int>(s);
    m_stateLabel->setText(QString("State: %1").arg(names[idx < 6 ? idx : 0]));
}

void MainWindow::updateConnectButton(QPushButton* btn, bool connected, const QString& baseName)
{
    // Gorsel artik temadan geliyor (QPushButton#connectBtn[state=...]).
    btn->setText(baseName + (connected ? "  Connected" : "  Connect"));
    btn->setProperty("state", connected ? "on" : "off");
    theme::repolish(btn);
}

void MainWindow::onMcuConnectionChanged(bool connected)
{
    updateConnectButton(m_mcuBtn, connected, "MCU");
    m_mcuBtn->setChecked(connected);
    m_mcuStatusLabel->setText(connected ? "MCU: Connected" : "MCU: Disconnected");
    m_mcuStatusLabel->setStyleSheet(connected ? "color:#2ecc71;font-size:11px;padding:0 6px;"
                                              : "color:#e74c3c;font-size:11px;padding:0 6px;");
}

void MainWindow::onLaserConnectionChanged(bool connected)
{
    // Baglanti denemesi bitti — butonu tekrar etkinlestir.
    m_laserBtn->setEnabled(true);
    updateConnectButton(m_laserBtn, connected, "Laser");
    m_laserBtn->setChecked(connected);
    m_laserStatusLabel->setText(connected ? "Laser: Connected" : "Laser: Disconnected");
    m_laserStatusLabel->setStyleSheet(connected ? "color:#2ecc71;font-size:11px;padding:0 6px;"
                                                : "color:#e74c3c;font-size:11px;padding:0 6px;");

    // Basari durumunda overlay'i guncelle. Hata durumu artik onLaserConnectFailed
    // tarafindan, gercek hata metniyle ele aliniyor.
    if (connected && m_connectingOverlay && m_connectingOverlay->isVisible())
        m_connectingOverlay->showSuccess();
}

void MainWindow::onLaserConnectStarted()
{
    // Arka plan baglanti denemesi suruyor: cift tiklamayi/yaris durumunu onlemek
    // icin butonu gecici olarak kilitle.
    m_laserBtn->setEnabled(false);
    m_laserStatusLabel->setText("Laser: Connecting...");
    m_laserStatusLabel->setStyleSheet("color:#f1c40f;font-size:11px;padding:0 6px;");
}

void MainWindow::onLaserConnectFailed(const QString& reason)
{
    // Butonu eski (bagli degil) duruma dondur ve tekrar etkinlestir.
    m_laserBtn->setEnabled(true);
    m_laserBtn->setChecked(false);
    updateConnectButton(m_laserBtn, false, "Laser");
    m_laserStatusLabel->setText("Laser: Disconnected");
    m_laserStatusLabel->setStyleSheet("color:#e74c3c;font-size:11px;padding:0 6px;");

    if (m_connectingOverlay)
        m_connectingOverlay->showError(reason);
}

void MainWindow::onProcessingStarted(const QString& taskName)
{
    if (!m_progressDialog) {
        m_progressDialog = new QProgressDialog(this);
        m_progressDialog->setWindowModality(Qt::WindowModal);
        m_progressDialog->setCancelButton(nullptr); // No cancel
        m_progressDialog->setMinimum(0);
        m_progressDialog->setMaximum(0); // Indeterminate
        m_progressDialog->setMinimumDuration(0); // Show immediately
    }
    m_progressDialog->setLabelText(taskName);
    m_progressDialog->show();
}

void MainWindow::onProcessingFinished()
{
    if (m_progressDialog) {
        m_progressDialog->hide();
    }
}
