#include "ProcessPanel.hpp"
#include "../controller/ScanController.hpp"
#include "../widgets/VisualizerWidget.hpp"

#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QCheckBox>

ProcessPanel::ProcessPanel(ScanController* controller, VisualizerWidget* viz, QWidget* parent)
    : QWidget(parent), m_controller(controller), m_viz(viz)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // Voxel Grid Filter
    QGroupBox* gbVoxel = new QGroupBox("Voxel Grid (Seyreltme)", this);
    QVBoxLayout* lVoxel = new QVBoxLayout(gbVoxel);
    QHBoxLayout* hVoxel = new QHBoxLayout();
    hVoxel->addWidget(new QLabel("Küp Boyutu (mm):", this));
    m_voxelLeafSpin = new QDoubleSpinBox(this);
    m_voxelLeafSpin->setRange(0.1, 50.0);
    m_voxelLeafSpin->setValue(1.0);
    m_voxelLeafSpin->setSingleStep(0.5);
    hVoxel->addWidget(m_voxelLeafSpin);
    lVoxel->addLayout(hVoxel);
    QPushButton* btnVoxel = new QPushButton("Seyrelt (Downsample)", this);
    lVoxel->addWidget(btnVoxel);
    mainLayout->addWidget(gbVoxel);

    // SOR Filter
    QGroupBox* gbSOR = new QGroupBox("Statistical Outlier Removal (SOR)", this);
    QVBoxLayout* lSOR = new QVBoxLayout(gbSOR);
    QHBoxLayout* hSOR1 = new QHBoxLayout();
    hSOR1->addWidget(new QLabel("Komşu Sayısı:", this));
    m_sorNeighborsSpin = new QSpinBox(this);
    m_sorNeighborsSpin->setRange(1, 200);
    m_sorNeighborsSpin->setValue(50);
    hSOR1->addWidget(m_sorNeighborsSpin);
    lSOR->addLayout(hSOR1);
    QHBoxLayout* hSOR2 = new QHBoxLayout();
    hSOR2->addWidget(new QLabel("Standart Sapma:", this));
    m_sorStdDevSpin = new QDoubleSpinBox(this);
    m_sorStdDevSpin->setRange(0.1, 10.0);
    m_sorStdDevSpin->setValue(1.0);
    m_sorStdDevSpin->setSingleStep(0.1);
    hSOR2->addWidget(m_sorStdDevSpin);
    lSOR->addLayout(hSOR2);
    QPushButton* btnSOR = new QPushButton("Gürültüyü Temizle (SOR)", this);
    lSOR->addWidget(btnSOR);
    mainLayout->addWidget(gbSOR);

    // ROR Filter
    QGroupBox* gbROR = new QGroupBox("Radius Outlier Removal (ROR)", this);
    QVBoxLayout* lROR = new QVBoxLayout(gbROR);
    QHBoxLayout* hROR1 = new QHBoxLayout();
    hROR1->addWidget(new QLabel("Yarıçap (mm):", this));
    m_rorRadiusSpin = new QDoubleSpinBox(this);
    m_rorRadiusSpin->setRange(0.1, 100.0);
    m_rorRadiusSpin->setValue(2.0);
    m_rorRadiusSpin->setSingleStep(0.5);
    hROR1->addWidget(m_rorRadiusSpin);
    lROR->addLayout(hROR1);
    QHBoxLayout* hROR2 = new QHBoxLayout();
    hROR2->addWidget(new QLabel("Min Nokta:", this));
    m_rorMinPtsSpin = new QSpinBox(this);
    m_rorMinPtsSpin->setRange(1, 200);
    m_rorMinPtsSpin->setValue(10);
    hROR2->addWidget(m_rorMinPtsSpin);
    lROR->addLayout(hROR2);
    QPushButton* btnROR = new QPushButton("Gürültüyü Temizle (ROR)", this);
    lROR->addWidget(btnROR);
    mainLayout->addWidget(gbROR);

    // Auto Filters (Cylindrical)
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

    QPushButton* m_btnAutoFilter = new QPushButton("Silindir Disini Sil", this);
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
    connect(btnVoxel, &QPushButton::clicked, this, &ProcessPanel::onApplyVoxelGridFilter);
    connect(btnSOR, &QPushButton::clicked, this, &ProcessPanel::onApplySORFilter);
    connect(btnROR, &QPushButton::clicked, this, &ProcessPanel::onApplyRORFilter);
    connect(m_btnAutoFilter, &QPushButton::clicked, this, &ProcessPanel::onApplyCylindricalFilter);
    connect(m_chkManualSelect, &QCheckBox::stateChanged, this, &ProcessPanel::onEnableManualSelection);
    connect(m_btnDeleteSelected, &QPushButton::clicked, this, &ProcessPanel::onDeleteSelected);
    connect(m_btnUndo, &QPushButton::clicked, this, &ProcessPanel::onUndo);
    connect(m_btnReset, &QPushButton::clicked, this, &ProcessPanel::onReset);

    connect(m_controller, &ScanController::historySizeChanged, this, &ProcessPanel::onHistorySizeChanged);
}

void ProcessPanel::onApplyCylindricalFilter()
{
    if (m_controller->getLastCloud().isEmpty() && m_viz->pointCount() > 0) {
        m_controller->setLastCloud(m_viz->getPoints());
    }

    float r = m_radiusSpin->value();
    float minZ = m_minZSpin->value();
    float maxZ = m_maxZSpin->value();
    m_controller->applyFilterCylindrical(r, minZ, maxZ);
}

void ProcessPanel::onApplySORFilter()
{
    if (m_controller->getLastCloud().isEmpty() && m_viz->pointCount() > 0) {
        m_controller->setLastCloud(m_viz->getPoints());
    }
    m_controller->applyFilterStatistical(m_sorNeighborsSpin->value(), m_sorStdDevSpin->value());
}

void ProcessPanel::onApplyRORFilter()
{
    if (m_controller->getLastCloud().isEmpty() && m_viz->pointCount() > 0) {
        m_controller->setLastCloud(m_viz->getPoints());
    }
    m_controller->applyFilterRadius(m_rorRadiusSpin->value(), m_rorMinPtsSpin->value());
}

void ProcessPanel::onApplyVoxelGridFilter()
{
    if (m_controller->getLastCloud().isEmpty() && m_viz->pointCount() > 0) {
        m_controller->setLastCloud(m_viz->getPoints());
    }
    m_controller->applyFilterVoxelGrid(m_voxelLeafSpin->value());
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
