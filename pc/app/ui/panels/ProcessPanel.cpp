#include "ProcessPanel.hpp"
#include "../controller/ScanController.hpp"
#include "../widgets/VisualizerWidget.hpp"

#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QCheckBox>

ProcessPanel::ProcessPanel(ScanController* controller, VisualizerWidget* viz, QWidget* parent)
    : QWidget(parent), m_controller(controller), m_viz(viz)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(15);

    // Auto Filters
    QGroupBox* gbAuto = new QGroupBox("Otomatik Filtreler (Silindir)", this);
    QVBoxLayout* lAuto = new QVBoxLayout(gbAuto);

    QHBoxLayout* hRad = new QHBoxLayout();
    hRad->addWidget(new QLabel("Yariçap (mm):", this));
    m_radiusSpin = new QDoubleSpinBox(this);
    m_radiusSpin->setRange(1.0, 500.0);
    m_radiusSpin->setValue(100.0);
    hRad->addWidget(m_radiusSpin);
    lAuto->addLayout(hRad);

    QHBoxLayout* hZ = new QHBoxLayout();
    hZ->addWidget(new QLabel("Min Z / Max Z:", this));
    m_minZSpin = new QDoubleSpinBox(this);
    m_minZSpin->setRange(-500.0, 500.0);
    m_minZSpin->setValue(-10.0);
    m_maxZSpin = new QDoubleSpinBox(this);
    m_maxZSpin->setRange(-500.0, 500.0);
    m_maxZSpin->setValue(200.0);
    hZ->addWidget(m_minZSpin);
    hZ->addWidget(m_maxZSpin);
    lAuto->addLayout(hZ);

    m_btnAutoFilter = new QPushButton("Silindir Disini Sil", this);
    lAuto->addWidget(m_btnAutoFilter);
    mainLayout->addWidget(gbAuto);

    // Manual Filters
    QGroupBox* gbManual = new QGroupBox("Manuel Seçim (MeshLab Tarzi)", this);
    QVBoxLayout* lManual = new QVBoxLayout(gbManual);
    
    m_chkManualSelect = new QCheckBox("Ekranda Fareyle Secim Yap", this);
    lManual->addWidget(m_chkManualSelect);

    m_btnDeleteSelected = new QPushButton("Secili Noktalari Sil", this);
    m_btnDeleteSelected->setEnabled(false);
    lManual->addWidget(m_btnDeleteSelected);
    mainLayout->addWidget(gbManual);

    // History Control
    QGroupBox* gbHistory = new QGroupBox("Gecmis", this);
    QVBoxLayout* lHistory = new QVBoxLayout(gbHistory);

    m_btnUndo = new QPushButton("Geri Al (Undo)", this);
    m_btnUndo->setEnabled(false);
    lHistory->addWidget(m_btnUndo);

    m_btnReset = new QPushButton("Orijinale Don", this);
    m_btnReset->setEnabled(false);
    lHistory->addWidget(m_btnReset);
    mainLayout->addWidget(gbHistory);

    mainLayout->addStretch();

    // Connections
    connect(m_btnAutoFilter, &QPushButton::clicked, this, &ProcessPanel::onApplyAutoFilter);
    connect(m_chkManualSelect, &QCheckBox::stateChanged, this, &ProcessPanel::onEnableManualSelection);
    connect(m_btnDeleteSelected, &QPushButton::clicked, this, &ProcessPanel::onDeleteSelected);
    connect(m_btnUndo, &QPushButton::clicked, this, &ProcessPanel::onUndo);
    connect(m_btnReset, &QPushButton::clicked, this, &ProcessPanel::onReset);

    connect(m_controller, &ScanController::historySizeChanged, this, &ProcessPanel::onHistorySizeChanged);
}

void ProcessPanel::onApplyAutoFilter()
{
    if (m_controller->getLastCloud().isEmpty() && m_viz->pointCount() > 0) {
        m_controller->setLastCloud(m_viz->getPoints());
    }

    float r = m_radiusSpin->value();
    float minZ = m_minZSpin->value();
    float maxZ = m_maxZSpin->value();
    m_controller->applyFilterCylindrical(r, minZ, maxZ);
}

void ProcessPanel::onEnableManualSelection(int state)
{
    bool enabled = (state == Qt::Checked);
    m_viz->setManualSelectionEnabled(enabled);
    m_btnDeleteSelected->setEnabled(enabled);
}

void ProcessPanel::onDeleteSelected()
{
    if (m_controller->getLastCloud().isEmpty() && m_viz->pointCount() > 0) {
        m_controller->setLastCloud(m_viz->getPoints());
    }

    QVector<int> selected = m_viz->getSelectedPointIndices();
    if (!selected.isEmpty()) {
        m_controller->applyManualDeletion(selected);
        m_viz->clearSelection();
    }
}

void ProcessPanel::onUndo()
{
    m_controller->undoLastFilter();
}

void ProcessPanel::onReset()
{
    m_controller->resetCloud();
}

void ProcessPanel::onHistorySizeChanged(int size)
{
    m_btnUndo->setEnabled(size > 0);
    m_btnReset->setEnabled(size > 0);
}
