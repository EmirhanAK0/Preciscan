#pragma once

#include <QWidget>

class QLabel;
class QProgressBar;
class QTimer;

class DiagnosticsPanel : public QWidget
{
    Q_OBJECT

public:
    explicit DiagnosticsPanel(QWidget* parent = nullptr);

private slots:
    void updateMetrics();

private:
    // System Health
    QLabel* m_laserStatus;
    QLabel* m_sdkStatus;
    QLabel* m_triggerStatus;
    QLabel* m_stateLabel;
    QProgressBar* m_bufferUsage;

    // Acquisition Quality
    QLabel* m_profileRate;
    QLabel* m_droppedProfiles;
    QLabel* m_plr;

    // Motion
    QLabel* m_rpmStatus;

    // Warnings
    QLabel* m_warningLine;

    QTimer* m_timer;
};
