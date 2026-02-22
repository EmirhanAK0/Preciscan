#include "ScanPanel.hpp"
#include "../../controller/ScanController.hpp"
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QTextEdit>
#include <QLabel>
#include <QFrame>
#include <QDateTime>
#include <QSlider>
#include <QSpinBox>
#include <QComboBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QFileDialog>

static QFrame* makeSep(QWidget* p) {
    auto* f = new QFrame(p);
    f->setFrameShape(QFrame::HLine);
    f->setStyleSheet("color: #2a2a2a; margin: 2px 0;");
    return f;
}

static QLabel* sectionLabel(const QString& t, QWidget* p) {
    auto* l = new QLabel(t, p);
    l->setStyleSheet("color: #555; font-size: 9px; font-weight: bold; text-transform: uppercase; letter-spacing: 1px; margin-top: 4px;");
    return l;
}

ScanPanel::ScanPanel(ScanController* ctrl, QWidget* parent)
    : QWidget(parent), m_ctrl(ctrl)
{
    setMinimumWidth(300);
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(10, 8, 10, 8);
    root->setSpacing(4);

    // ── Ayarlar ─────────────────────────────────────────────────
    root->addWidget(sectionLabel("AYARLAR", this));

    auto* grid = new QGridLayout;
    grid->setHorizontalSpacing(8);
    grid->setVerticalSpacing(4);

    auto* speedLabel = new QLabel("Hiz (rps):", this);
    speedLabel->setStyleSheet("color: #aaa; font-size: 10px;");
    m_speedSlider = new QSlider(Qt::Horizontal, this);
    m_speedSlider->setRange(5, 70); m_speedSlider->setValue(10);
    m_speedSpin = new QSpinBox(this);
    m_speedSpin->setRange(5, 70); m_speedSpin->setValue(10);
    m_speedSpin->setFixedWidth(55);
    m_speedSpin->setStyleSheet("background:#1a1a1a; color:#ccc; border:1px solid #333; border-radius:3px;");
    connect(m_speedSlider, &QSlider::valueChanged, m_speedSpin, &QSpinBox::setValue);
    connect(m_speedSpin, QOverload<int>::of(&QSpinBox::valueChanged), m_speedSlider, &QSlider::setValue);
    connect(m_speedSlider, &QSlider::valueChanged, this, [this](int val){
        if (m_ctrl) m_ctrl->setRps(static_cast<float>(val));
    });

    auto* expLabel = new QLabel("Poz. (us):", this);
    expLabel->setStyleSheet("color: #aaa; font-size: 10px;");
    m_exposureSlider = new QSlider(Qt::Horizontal, this);
    m_exposureSlider->setRange(50, 5000); m_exposureSlider->setValue(500);
    m_exposureSpin = new QSpinBox(this);
    m_exposureSpin->setRange(50, 5000); m_exposureSpin->setValue(500);
    m_exposureSpin->setFixedWidth(55);
    m_exposureSpin->setStyleSheet("background:#1a1a1a; color:#ccc; border:1px solid #333; border-radius:3px;");
    connect(m_exposureSlider, &QSlider::valueChanged, m_exposureSpin, &QSpinBox::setValue);
    connect(m_exposureSpin, QOverload<int>::of(&QSpinBox::valueChanged), m_exposureSlider, &QSlider::setValue);

    grid->addWidget(speedLabel,       0, 0);
    grid->addWidget(m_speedSlider,    0, 1);
    grid->addWidget(m_speedSpin,      0, 2);
    grid->addWidget(expLabel,         1, 0);
    grid->addWidget(m_exposureSlider, 1, 1);
    grid->addWidget(m_exposureSpin,   1, 2);
    root->addLayout(grid);

    // ── Kontrol ──────────────────────────────────────────────────
    root->addWidget(makeSep(this));
    root->addWidget(sectionLabel("KONTROL", this));

    m_scanStatusLabel = new QLabel("Beklemede", this);
    m_scanStatusLabel->setAlignment(Qt::AlignCenter);
    m_scanStatusLabel->setStyleSheet("color:#777; font-size:10px; padding:2px; background:#111; border-radius:3px;");
    root->addWidget(m_scanStatusLabel);

    auto* btnRow = new QHBoxLayout;
    m_startBtn = new QPushButton("TARAMAYI BASLAT", this);
    m_startBtn->setEnabled(false);
    m_startBtn->setStyleSheet("QPushButton{background:#1a3a1a;color:#3a3;font-weight:bold;border:1px solid #2a5;border-radius:4px;padding:7px;} QPushButton:enabled{background:#1c4a1c;color:#4e4;} QPushButton:hover:enabled{background:#2ecc71;color:#000;} QPushButton:disabled{background:#1a1a1a;color:#444;border-color:#333;}");

    m_stopBtn = new QPushButton("DURDUR", this);
    m_stopBtn->setEnabled(false);
    m_stopBtn->setFixedWidth(90);
    m_stopBtn->setStyleSheet("QPushButton{background:#3a1a1a;color:#a33;font-weight:bold;border:1px solid #a33;border-radius:4px;padding:7px;} QPushButton:enabled{background:#4a1a1a;color:#e44;} QPushButton:hover:enabled{background:#e74c3c;color:#fff;} QPushButton:disabled{background:#1a1a1a;color:#444;border-color:#333;}");

    btnRow->addWidget(m_startBtn, 1);
    btnRow->addWidget(m_stopBtn, 0);
    root->addLayout(btnRow);

    // ── Katmanlar ─────────────────────────────────────────────────
    root->addWidget(makeSep(this));
    root->addWidget(sectionLabel("KATMANLAR", this));

    m_layerList = new QListWidget(this);
    m_layerList->setMaximumHeight(110);
    m_layerList->setStyleSheet("QListWidget{background:#0d0d0d;color:#ccc;border:1px solid #2a2a2a;border-radius:3px;font-size:10px;} QListWidget::item:selected{background:#1a3a5a;color:#5af;}");
    m_layerList->setSelectionMode(QAbstractItemView::MultiSelection);
    root->addWidget(m_layerList);

    auto* mergeRow = new QHBoxLayout;
    m_mergeMode = new QComboBox(this);
    m_mergeMode->addItems({"Ortalama", "Maksimum", "Birlestir"});
    m_mergeMode->setStyleSheet("background:#1a1a1a;color:#ccc;border:1px solid #333;border-radius:3px;padding:2px;font-size:10px;");
    m_mergeBtn = new QPushButton("Birlestir", this);
    m_mergeBtn->setFixedWidth(80);
    m_mergeBtn->setStyleSheet("QPushButton{background:#1a2a3a;color:#5af;border:1px solid #25f;border-radius:3px;padding:4px;font-size:10px;} QPushButton:hover{background:#1e3a5a;}");
    m_addLayerBtn = new QPushButton("+ Sil", this); // Simüle: STL Seç
    m_addLayerBtn->setText("STL Sec");
    m_addLayerBtn->setFixedWidth(65);
    m_addLayerBtn->setStyleSheet("QPushButton{background:#222;color:#777;border:1px solid #333;border-radius:3px;padding:4px;font-size:9px;} QPushButton:hover{color:#aaa;}");
    mergeRow->addWidget(m_mergeMode, 1);
    mergeRow->addWidget(m_mergeBtn, 0);
    mergeRow->addWidget(m_addLayerBtn, 0);
    root->addLayout(mergeRow);

    // ── Log ───────────────────────────────────────────────────────
    root->addWidget(makeSep(this));
    root->addWidget(sectionLabel("LOG", this));

    m_logView = new QTextEdit(this);
    m_logView->setReadOnly(true);
    m_logView->setStyleSheet("background:#0a0a0a;color:#00cc66;font-family:Consolas,monospace;font-size:9px;border:1px solid #1a2a1a;border-radius:3px;");
    root->addWidget(m_logView, 1);

    // ── Baglanti ─────────────────────────────────────────────────
    connect(m_startBtn,   &QPushButton::clicked, this, &ScanPanel::onStartClicked);
    connect(m_stopBtn,    &QPushButton::clicked, this, &ScanPanel::onStopClicked);
    connect(m_mergeBtn,   &QPushButton::clicked, this, &ScanPanel::onMergeClicked);
    connect(m_addLayerBtn,&QPushButton::clicked, this, [this](){
        QString path = QFileDialog::getOpenFileName(this, "STL Dosyasi Sec", "", "STL Files (*.stl)");
        if (!path.isEmpty() && m_ctrl) {
            m_ctrl->connectLaserSim(path);
        }
    });

    if (m_ctrl) {
        connect(m_ctrl, &ScanController::scanStarted,          this, &ScanPanel::onScanStarted);
        connect(m_ctrl, &ScanController::scanStopped,          this, &ScanPanel::onScanStopped);
        connect(m_ctrl, &ScanController::mcuConnectionChanged,  this, &ScanPanel::onMcuConnected);
        connect(m_ctrl, &ScanController::laserConnectionChanged,this, &ScanPanel::onLaserConnected);
        connect(m_ctrl, &ScanController::logMessage, this, &ScanPanel::appendLog);
    }

    appendLog("SYS", "ScanPanel hazir.");
}

void ScanPanel::onStartClicked() { if (m_ctrl) m_ctrl->startScan(); }
void ScanPanel::onStopClicked()  { if (m_ctrl) m_ctrl->stopScan();  }

void ScanPanel::onScanStarted() {
    m_startBtn->setEnabled(false);
    m_stopBtn->setEnabled(true);
    m_scanStatusLabel->setText("TARAMA AKTIF");
    m_scanStatusLabel->setStyleSheet("color:#2ecc71;font-weight:bold;font-size:10px;padding:2px;background:#0d1f15;border-radius:3px;");
}

void ScanPanel::onScanStopped() {
    m_startBtn->setEnabled(m_mcuReady || m_laserReady);
    m_stopBtn->setEnabled(false);
    m_scanStatusLabel->setText("Durduruldu");
    m_scanStatusLabel->setStyleSheet("color:#aaa;font-size:10px;padding:2px;background:#111;border-radius:3px;");
}

void ScanPanel::onMcuConnected(bool ok) {
    m_mcuReady = ok;
    appendLog(ok ? "OK" : "ERR", ok ? "MCU baglandi." : "MCU baglanamadi!");
    updateStartButtonState();
}

void ScanPanel::onLaserConnected(bool ok) {
    m_laserReady = ok;
    // appendLog ScanController'dan geliyor
    updateStartButtonState();
}

void ScanPanel::updateStartButtonState() {
    bool canScan = m_mcuReady || m_laserReady;
    m_startBtn->setEnabled(canScan && !m_ctrl->isScanning());
}

void ScanPanel::onMergeClicked() {
    auto items = m_layerList->selectedItems();
    if (items.isEmpty()) { appendLog("WARN","Birlestirilecek katman secilmedi."); return; }
    QString mode = m_mergeMode->currentText();
    appendLog("OK", QString("%1 katman birlestiriliyor. Mod: %2").arg(items.size()).arg(mode));
}

void ScanPanel::onAddLayerDemo() {
    // Artik kullanilmiyor (STL Sec oldu)
}

void ScanPanel::appendLog(const QString& level, const QString& msg) {
    const QString ts = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString color = (level=="ERR"||level=="WARN") ? "#e74c3c" : (level=="OK" ? "#2ecc71" : "#888");
    m_logView->append(QString("<span style='color:#555'>[%1]</span> <span style='color:%2'>[%3]</span> %4")
                      .arg(ts, color, level, msg));
}