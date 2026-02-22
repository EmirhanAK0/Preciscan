#include "MainWindow.hpp"
#include "state/AppStateMachine.hpp"
#include "panels/ScanPanel.hpp"
#include "panels/SetupPanel.hpp"
#include "panels/DiagnosticsPanel.hpp"
#include "widgets/VisualizerWidget.hpp"
#include "../controller/ScanController.hpp"

#include <QFileDialog>
#include <QMessageBox>
#include <QToolBar>
#include <QTabWidget>
#include <QStatusBar>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>

static QLabel* dotLabel(const QString& txt, const QString& color, QWidget* p) {
    auto* l = new QLabel(txt, p);
    l->setStyleSheet(QString("color:%1; font-size:11px; padding:0 6px;").arg(color));
    return l;
}

MainWindow::MainWindow(McuListener* mcu, LaserManager* laser,
                       SPSCRingBuffer* ring, QWidget* parent)
    : QMainWindow(parent)
{
    m_stateMachine   = new AppStateMachine(this);
    m_scanController = new ScanController(mcu, laser, ring, this);

    setWindowTitle("Preciscan");
    resize(1200, 750);

    setupToolBar();
    setupCentralWidget();
    setupStatusBar();

    connect(m_stateMachine,   &AppStateMachine::stateChanged,
            this, &MainWindow::onStateChanged);
    connect(m_scanController, &ScanController::scanStarted,
            this, [this]{ m_stateMachine->setState(AppState::Scanning); });
    connect(m_scanController, &ScanController::scanStopped,
            this, [this]{ m_stateMachine->setState(AppState::Idle); });

    connect(m_scanController, &ScanController::mcuConnectionChanged,
            this, &MainWindow::onMcuConnectionChanged);
    connect(m_scanController, &ScanController::laserConnectionChanged,
            this, &MainWindow::onLaserConnectionChanged);

    onStateChanged(m_stateMachine->currentState());
    onMcuConnectionChanged(false);
    onLaserConnectionChanged(false);
}

void MainWindow::setupToolBar() {
    auto* tb = addToolBar("Hardware");
    tb->setMovable(false);
    tb->setStyleSheet(
        "QToolBar { background: #111; border-bottom: 1px solid #222; spacing: 4px; padding: 4px 8px; }"
        "QPushButton { border-radius: 4px; font-size: 10px; font-weight: bold; padding: 5px 12px; min-width: 90px; }"
    );

    m_mcuBtn = new QPushButton("MCU  Baglan", this);
    m_mcuBtn->setCheckable(true);
    connect(m_mcuBtn, &QPushButton::clicked, this, [this](bool checked) {
        if (checked) m_scanController->connectMcu();
        else         m_scanController->disconnectMcu();
    });
    tb->addWidget(m_mcuBtn);

    auto* spacerL = new QWidget; spacerL->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tb->addWidget(spacerL);

    auto* title = new QLabel("PRECISCAN", this);
    title->setStyleSheet("color:#eee; font-size:14px; font-weight:bold; letter-spacing:3px;");
    title->setAlignment(Qt::AlignCenter);
    tb->addWidget(title);

    auto* spacerR = new QWidget; spacerR->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tb->addWidget(spacerR);

    m_laserBtn = new QPushButton("Lazer  Baglan", this);
    m_laserBtn->setCheckable(true);
    connect(m_laserBtn, &QPushButton::clicked, this, [this](bool checked) {
        if (checked) m_scanController->connectLaser();
        else         m_scanController->disconnectLaser();
    });
    tb->addWidget(m_laserBtn);
}

void MainWindow::setupCentralWidget() {
    auto* central = new QWidget(this);
    setCentralWidget(central);
    auto* hLayout = new QHBoxLayout(central);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);

    auto* viz = new VisualizerWidget(this);
    viz->setMinimumWidth(400);

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setFixedWidth(360);
    m_tabWidget->setStyleSheet(
        "QTabWidget::pane { border: none; background: #141414; }"
        "QTabBar::tab { background:#1a1a1a; color:#777; padding:6px 14px; font-size:10px; font-weight:bold; border:none; }"
        "QTabBar::tab:selected { background:#141414; color:#eee; border-bottom:2px solid #2ecc71; }"
        "QTabBar::tab:hover { color:#bbb; }"
    );

    auto* scanPanel  = new ScanPanel(m_scanController, this);
    auto* setupPanel = new SetupPanel(m_scanController, this);
    auto* diagPanel  = new DiagnosticsPanel(this);

    m_tabWidget->addTab(scanPanel,  "Scan");
    m_tabWidget->addTab(setupPanel, "Setup");
    m_tabWidget->addTab(diagPanel,  "Diagnostics");

    // Sinyal baglantilari
    connect(m_scanController, &ScanController::mcuConnectionChanged,
            scanPanel, &ScanPanel::onMcuConnected);
    connect(m_scanController, &ScanController::laserConnectionChanged,
            scanPanel, &ScanPanel::onLaserConnected);

    // Simulation signals
    connect(m_scanController, &ScanController::simProfileReceived,
            viz, [viz](float theta, const QVector<QPointF>& profile) {
        viz->addProfile(theta, profile);
    });

    // Mesh ve Temizleme baglantilari
    connect(m_scanController, &ScanController::meshLoaded,
            viz, &VisualizerWidget::setMesh);
    connect(m_scanController, &ScanController::requestClearVisualizer,
            viz, &VisualizerWidget::clearPoints);
    
    connect(m_scanController, &ScanController::pointCloudReady,
            this, [this, viz](const QVector<QVector3D>& cloud) {
        viz->clearPoints();
        viz->addPoints(cloud);
        
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Tarama Tamamlandi");
        msgBox.setText(QString("Tarama tamamlandi (%1 nokta). PLY olarak kaydetmek ister misiniz?").arg(cloud.size()));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        
        if (msgBox.exec() == QMessageBox::Yes) {
            QString path = QFileDialog::getSaveFileName(this, "Kaydet", "", "PLY Files (*.ply)");
            if (!path.isEmpty()) m_scanController->saveCurrentScan(path);
        }
    });

    // MCU log seyreltme
    static int mcuCounter = 0;
    connect(m_scanController, &ScanController::mcuPacketReceived,
            this, [scanPanel](quint32 seq, float y) {
        if (++mcuCounter >= 100) {
            mcuCounter = 0;
            scanPanel->appendLog("MCU", QString("Sinyal Aliniyor... Tetik:%1 Y:%2 mm").arg(seq).arg(y, 0, 'f', 2));
        }
    });

    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(viz);
    splitter->addWidget(m_tabWidget);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);
    splitter->setHandleWidth(2);
    splitter->setStyleSheet("QSplitter::handle { background: #222; }");

    hLayout->addWidget(splitter);
}

void MainWindow::setupStatusBar() {
    auto* sb = statusBar();
    sb->setStyleSheet("QStatusBar { background:#0d0d0d; color:#555; font-size:10px; border-top:1px solid #222; }");

    m_stateLabel = dotLabel("State: Idle", "#888", this);
    sb->addWidget(m_stateLabel);

    sb->addWidget(dotLabel("|", "#333", this));
    m_mcuStatusLabel = dotLabel("MCU: Bagli degil", "#e74c3c", this);
    sb->addWidget(m_mcuStatusLabel);

    sb->addWidget(dotLabel("|", "#333", this));
    m_laserStatusLabel = dotLabel("Lazer: Bagli degil", "#e74c3c", this);
    sb->addWidget(m_laserStatusLabel);

    sb->addPermanentWidget(dotLabel("Preciscan v2.1", "#333", this));
}

void MainWindow::onStateChanged(AppState s) {
    static const char* names[] = {"Idle","Ready","Scanning","Paused","Completed","Fault"};
    int idx = static_cast<int>(s);
    m_stateLabel->setText(QString("State: %1").arg(names[idx < 6 ? idx : 0]));
}

void MainWindow::updateConnectButton(QPushButton* btn, bool connected,
                                     const QString& baseName) {
    if (connected) {
        btn->setText(baseName + "  Baglidir");
        btn->setStyleSheet("QPushButton{background:#0d2a0d;color:#2ecc71;border:1px solid #2ecc71;border-radius:4px;font-size:10px;font-weight:bold;padding:5px 12px;} QPushButton:hover{background:#0d3a0d;}");
    } else {
        btn->setText(baseName + "  Baglan");
        btn->setStyleSheet("QPushButton{background:#1e1e1e;color:#888;border:1px solid #444;border-radius:4px;font-size:10px;font-weight:bold;padding:5px 12px;} QPushButton:hover{background:#2a2a2a;color:#bbb;}");
    }
}

void MainWindow::onMcuConnectionChanged(bool connected) {
    updateConnectButton(m_mcuBtn, connected, "MCU");
    m_mcuBtn->setChecked(connected);
    m_mcuStatusLabel->setText(connected ? "MCU: Bagli" : "MCU: Bagli degil");
    m_mcuStatusLabel->setStyleSheet(
        connected ? "color:#2ecc71;font-size:11px;padding:0 6px;"
                  : "color:#e74c3c;font-size:11px;padding:0 6px;");
}

void MainWindow::onLaserConnectionChanged(bool connected) {
    updateConnectButton(m_laserBtn, connected, "Lazer");
    m_laserBtn->setChecked(connected);
    m_laserStatusLabel->setText(connected ? "Lazer: Bagli" : "Lazer: Bagli degil");
    m_laserStatusLabel->setStyleSheet(
        connected ? "color:#2ecc71;font-size:11px;padding:0 6px;"
                  : "color:#e74c3c;font-size:11px;padding:0 6px;");
}
