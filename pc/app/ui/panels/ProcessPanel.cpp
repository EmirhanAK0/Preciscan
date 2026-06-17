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
#include <QFileDialog>
#include <QMessageBox>
#include <QFrame>
#include "../../../io/ply_writer.h"

static QFrame* makeSep(QWidget* p) {
    QFrame* f = new QFrame(p);
    f->setFrameShape(QFrame::HLine);
    f->setFrameShadow(QFrame::Sunken);
    return f;
}

ProcessPanel::ProcessPanel(ScanController* controller, VisualizerWidget* viz, QWidget* parent)
    : QWidget(parent), m_controller(controller), m_viz(viz)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // Voxel Grid Filter
    QGroupBox* gbVoxel = new QGroupBox("Voxel Grid (Downsample)", this);
    QVBoxLayout* lVoxel = new QVBoxLayout(gbVoxel);
    QHBoxLayout* hVoxel = new QHBoxLayout();
    hVoxel->addWidget(new QLabel("Cube Size (mm):", this));
    m_voxelLeafSpin = new QDoubleSpinBox(this);
    m_voxelLeafSpin->setRange(0.1, 50.0);
    m_voxelLeafSpin->setValue(1.0);
    m_voxelLeafSpin->setSingleStep(0.5);
    hVoxel->addWidget(m_voxelLeafSpin);
    lVoxel->addLayout(hVoxel);
    QPushButton* btnVoxel = new QPushButton("Downsample", this);
    lVoxel->addWidget(btnVoxel);
    mainLayout->addWidget(gbVoxel);

    // SOR Filter
    QGroupBox* gbSOR = new QGroupBox("Statistical Outlier Removal (SOR)", this);
    QVBoxLayout* lSOR = new QVBoxLayout(gbSOR);
    QHBoxLayout* hSOR1 = new QHBoxLayout();
    hSOR1->addWidget(new QLabel("Neighbor Count:", this));
    m_sorNeighborsSpin = new QSpinBox(this);
    m_sorNeighborsSpin->setRange(1, 200);
    m_sorNeighborsSpin->setValue(50);
    hSOR1->addWidget(m_sorNeighborsSpin);
    lSOR->addLayout(hSOR1);
    QHBoxLayout* hSOR2 = new QHBoxLayout();
    hSOR2->addWidget(new QLabel("Std Deviation:", this));
    m_sorStdDevSpin = new QDoubleSpinBox(this);
    m_sorStdDevSpin->setRange(0.1, 10.0);
    m_sorStdDevSpin->setValue(1.0);
    m_sorStdDevSpin->setSingleStep(0.1);
    hSOR2->addWidget(m_sorStdDevSpin);
    lSOR->addLayout(hSOR2);
    QPushButton* btnSOR = new QPushButton("Remove Noise (SOR)", this);
    lSOR->addWidget(btnSOR);
    mainLayout->addWidget(gbSOR);

    // ROR Filter
    QGroupBox* gbROR = new QGroupBox("Radius Outlier Removal (ROR)", this);
    QVBoxLayout* lROR = new QVBoxLayout(gbROR);
    QHBoxLayout* hROR1 = new QHBoxLayout();
    hROR1->addWidget(new QLabel("Radius (mm):", this));
    m_rorRadiusSpin = new QDoubleSpinBox(this);
    m_rorRadiusSpin->setRange(0.1, 100.0);
    m_rorRadiusSpin->setValue(2.0);
    m_rorRadiusSpin->setSingleStep(0.5);
    hROR1->addWidget(m_rorRadiusSpin);
    lROR->addLayout(hROR1);
    QHBoxLayout* hROR2 = new QHBoxLayout();
    hROR2->addWidget(new QLabel("Min Points:", this));
    m_rorMinPtsSpin = new QSpinBox(this);
    m_rorMinPtsSpin->setRange(1, 200);
    m_rorMinPtsSpin->setValue(10);
    hROR2->addWidget(m_rorMinPtsSpin);
    lROR->addLayout(hROR2);
    QPushButton* btnROR = new QPushButton("Remove Noise (ROR)", this);
    lROR->addWidget(btnROR);
    mainLayout->addWidget(gbROR);

    // Auto Filters (Cylindrical)
    QGroupBox* gbAuto = new QGroupBox("Automatic Filters (Cylinder)", this);
    QVBoxLayout* lAuto = new QVBoxLayout(gbAuto);

    QHBoxLayout* hRad = new QHBoxLayout();
    hRad->addWidget(new QLabel("Radius (mm):", this));
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

    QPushButton* m_btnAutoFilter = new QPushButton("Remove Outside Cylinder", this);
    lAuto->addWidget(m_btnAutoFilter);
    mainLayout->addWidget(gbAuto);

    // Manual Filters
    QGroupBox* gbManual = new QGroupBox("Manual Selection (MeshLab Style)", this);
    QVBoxLayout* lManual = new QVBoxLayout(gbManual);

    m_chkManualSelect = new QCheckBox("Select with Mouse on Screen", this);
    lManual->addWidget(m_chkManualSelect);

    m_btnDeleteSelected = new QPushButton("Delete Selected Points", this);
    m_btnDeleteSelected->setProperty("tier", "destructive");
    m_btnDeleteSelected->setEnabled(false);
    lManual->addWidget(m_btnDeleteSelected);
    mainLayout->addWidget(gbManual);

    // Mesh Generation
    QGroupBox* gbMesh = new QGroupBox("Surface Generation (Mesh)", this);
    QVBoxLayout* lMesh = new QVBoxLayout(gbMesh);

    m_btnGenerateMesh = new QPushButton("Generate Quick Surface (Mesh)", this);
    lMesh->addWidget(m_btnGenerateMesh);

    m_btnSaveMesh = new QPushButton("Save Surface (.obj)", this);
    lMesh->addWidget(m_btnSaveMesh);

    m_btnClearMesh = new QPushButton("Clear Surface", this);
    lMesh->addWidget(m_btnClearMesh);

    lMesh->addWidget(makeSep(this));

    m_btnLoadPLY = new QPushButton("Load PLY", this);
    lMesh->addWidget(m_btnLoadPLY);

    m_btnExportPLY = new QPushButton("Save Active (Filtered) Points (.ply)", this);
    lMesh->addWidget(m_btnExportPLY);
    
    mainLayout->addWidget(gbMesh);

    // History Control
    QGroupBox* gbHistory = new QGroupBox("History", this);
    QVBoxLayout* lHistory = new QVBoxLayout(gbHistory);

    m_btnUndo = new QPushButton("Undo", this);
    m_btnUndo->setEnabled(false);
    lHistory->addWidget(m_btnUndo);

    m_btnReset = new QPushButton("Revert to Original", this);
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
    connect(m_btnGenerateMesh, &QPushButton::clicked, this, &ProcessPanel::onGenerateMesh);
    connect(m_btnSaveMesh, &QPushButton::clicked, this, &ProcessPanel::onSaveMesh);
    connect(m_btnClearMesh, &QPushButton::clicked, this, &ProcessPanel::onClearMesh);
    connect(m_btnLoadPLY, &QPushButton::clicked, this, &ProcessPanel::onLoadPLY);
    connect(m_btnExportPLY, &QPushButton::clicked, this, &ProcessPanel::onExportPLY);

    connect(m_btnUndo, &QPushButton::clicked, this, &ProcessPanel::onUndo);
    connect(m_btnReset, &QPushButton::clicked, this, &ProcessPanel::onReset);

    connect(m_controller, &ScanController::historySizeChanged, this, &ProcessPanel::onHistorySizeChanged);
}

void ProcessPanel::onApplyCylindricalFilter()
{
    float r = m_radiusSpin->value();
    float minZ = m_minZSpin->value();
    float maxZ = m_maxZSpin->value();
    m_controller->applyFilterCylindrical(r, minZ, maxZ);
}

void ProcessPanel::onApplySORFilter()
{
    m_controller->applyFilterStatistical(m_sorNeighborsSpin->value(), m_sorStdDevSpin->value());
}

void ProcessPanel::onApplyRORFilter()
{
    m_controller->applyFilterRadius(m_rorRadiusSpin->value(), m_rorMinPtsSpin->value());
}

void ProcessPanel::onApplyVoxelGridFilter()
{
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

void ProcessPanel::onGenerateMesh()
{
    m_controller->generateMeshAsync();
}

void ProcessPanel::onClearMesh()
{
    m_controller->clearMesh();
}


void ProcessPanel::onSaveMesh()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save Mesh", "", "OBJ Files (*.obj)");
    if (fileName.isEmpty()) return;

    if (m_viz->saveMeshToOBJ(fileName)) {
        QMessageBox::information(this, "Success", "Mesh saved successfully.");
    } else {
        QMessageBox::critical(this, "Error", "Could not save mesh! Please generate a mesh first.");
    }
}


void ProcessPanel::onLoadPLY()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load PLY", "", "PLY Files (*.ply)");
    if (fileName.isEmpty()) return;

    QVector<QVector3D> points;
    if (io::readPLY(fileName, points)) {
        m_controller->addNewLayer(points, QFileInfo(fileName).baseName());
        QMessageBox::information(this, "Success", QString("%1 points loaded.").arg(points.size()));
    } else {
        QMessageBox::critical(this, "Error", "Could not read PLY file!");
    }
}

void ProcessPanel::onExportPLY()
{
    const QVector<QVector3D>& points = m_viz->getPoints();
    if (points.isEmpty()) {
        QMessageBox::warning(this, "Warning", "No points to export!");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Save Filtered PLY", "", "PLY Files (*.ply)");
    if (fileName.isEmpty()) return;

    // Normalli kaydet: MeshLab'da dogrudan Poisson mesh kurmayi saglar
    const QVector<QVector3D> normals = core::PointCloudProcessor::computeNormals(points);
    if (io::writePLYWithNormals(fileName, points, normals)) {
        QMessageBox::information(this, "Success",
            QString("%1 points saved successfully (%2).")
                .arg(points.size())
                .arg(normals.isEmpty() ? "without normals" : "with normals"));
    } else {
        QMessageBox::critical(this, "Error", "An error occurred while saving the file!");
    }
}
