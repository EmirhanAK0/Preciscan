#pragma once

#include <QWidget>

class ScanController;
class VisualizerWidget;
class QPushButton;
class QDoubleSpinBox;
class QSpinBox;
class QCheckBox;

class ProcessPanel : public QWidget {
    Q_OBJECT
public:
    explicit ProcessPanel(ScanController* controller, VisualizerWidget* viz, QWidget* parent = nullptr);

public slots:
    void onHistorySizeChanged(int size);

private slots:
    void onApplyCylindricalFilter();
    void onApplySORFilter();
    void onApplyRORFilter();
    void onApplyVoxelGridFilter();
    void onDeleteSelected();
    void onEnableManualSelection(int state);
    void onUndo();
    void onReset();
    void onGenerateMesh();
    void onSaveMesh();
    void onClearMesh();
    void onLoadPLY();
    void onExportPLY();

private:
    ScanController* m_controller;
    VisualizerWidget* m_viz;

    // Cylindrical
    QDoubleSpinBox* m_radiusSpin;
    QDoubleSpinBox* m_minZSpin;
    QDoubleSpinBox* m_maxZSpin;

    // SOR
    QSpinBox* m_sorNeighborsSpin;
    QDoubleSpinBox* m_sorStdDevSpin;

    // ROR
    QDoubleSpinBox* m_rorRadiusSpin;
    QSpinBox* m_rorMinPtsSpin;

    // Voxel Grid
    QDoubleSpinBox* m_voxelLeafSpin;

    QCheckBox* m_chkManualSelect;
    QPushButton* m_btnDeleteSelected;

    QPushButton* m_btnUndo;
    QPushButton* m_btnReset;

    QPushButton* m_btnGenerateMesh;
    QPushButton* m_btnSaveMesh;
    QPushButton* m_btnClearMesh;

    QPushButton* m_btnLoadPLY;
    QPushButton* m_btnExportPLY;
};
