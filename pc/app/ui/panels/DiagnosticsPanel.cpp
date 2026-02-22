#include "DiagnosticsPanel.hpp"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QProgressBar>
#include <QFrame>
#include <QTimer>

// -------- Section helper --------
static QFrame* createSection(const QString& title, QVBoxLayout*& outLayout)
{
    QFrame* frame = new QFrame;
    frame->setFrameShape(QFrame::StyledPanel);

    outLayout = new QVBoxLayout(frame);

    QLabel* header = new QLabel(title);
    header->setStyleSheet("font-weight: bold; font-size: 14px;");
    outLayout->addWidget(header);

    return frame;
}
// --------------------------------

DiagnosticsPanel::DiagnosticsPanel(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // ================= SYSTEM HEALTH =================
    QVBoxLayout* systemLayout;
    QFrame* systemFrame = createSection("System Health", systemLayout);

    QGridLayout* systemGrid = new QGridLayout;

    m_laserStatus = new QLabel("Laser: Connected");
    m_sdkStatus = new QLabel("SDK: OK");
    m_triggerStatus = new QLabel("Trigger: Sync OK");
    m_stateLabel = new QLabel("State: IDLE");

    m_bufferUsage = new QProgressBar;
    m_bufferUsage->setRange(0, 100);
    m_bufferUsage->setValue(20);

    systemGrid->addWidget(m_laserStatus, 0, 0);
    systemGrid->addWidget(m_sdkStatus, 0, 1);
    systemGrid->addWidget(m_triggerStatus, 1, 0);
    systemGrid->addWidget(m_stateLabel, 1, 1);
    systemGrid->addWidget(new QLabel("Buffer Usage:"), 2, 0);
    systemGrid->addWidget(m_bufferUsage, 2, 1);

    systemLayout->addLayout(systemGrid);

    // ================= ACQUISITION QUALITY =================
    QVBoxLayout* qualityLayout;
    QFrame* qualityFrame = createSection("Acquisition Quality", qualityLayout);

    QGridLayout* qualityGrid = new QGridLayout;

    m_profileRate = new QLabel("Profile Rate: 980 Hz");
    m_droppedProfiles = new QLabel("Dropped Profiles: 1.2 %");
    m_plr = new QLabel("PLR: 1.2 % (GOOD)");

    qualityGrid->addWidget(m_profileRate, 0, 0);
    qualityGrid->addWidget(m_plr, 0, 1);
    qualityGrid->addWidget(m_droppedProfiles, 1, 0);

    qualityLayout->addLayout(qualityGrid);

    // ================= MOTION =================
    QVBoxLayout* motionLayout;
    QFrame* motionFrame = createSection("Motion", motionLayout);

    QGridLayout* motionGrid = new QGridLayout;

    m_rpmStatus = new QLabel("RPM: 120 (cmd) / 118.7 (meas)");

    motionGrid->addWidget(m_rpmStatus, 0, 0);
    motionLayout->addLayout(motionGrid);

    // ================= WARNINGS =================
    QVBoxLayout* warningLayout;
    QFrame* warningFrame = createSection("Warnings", warningLayout);

    m_warningLine = new QLabel("[INFO] System operating normally");
    m_warningLine->setStyleSheet("color: orange;");

    warningLayout->addWidget(m_warningLine);

    // ================= MAIN =================
    mainLayout->addWidget(systemFrame);
    mainLayout->addWidget(qualityFrame);
    mainLayout->addWidget(motionFrame);
    mainLayout->addWidget(warningFrame);
    mainLayout->addStretch();

    // ================= TIMER =================
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DiagnosticsPanel::updateMetrics);
    m_timer->start(500);
}

void DiagnosticsPanel::updateMetrics()
{
    int buf = (m_bufferUsage->value() + 2) % 100;
    m_bufferUsage->setValue(buf);

    if (buf > 80) {
        m_warningLine->setText("[WARN] Buffer usage high");
        m_warningLine->setStyleSheet("color: red;");
    }
    else {
        m_warningLine->setText("[INFO] System operating normally");
        m_warningLine->setStyleSheet("color: orange;");
    }
}
