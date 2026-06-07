#pragma once

#include <QWidget>

class ScanController;
class VisualizerWidget;
class QPushButton;
class QDoubleSpinBox;
class QCheckBox;

class ProcessPanel : public QWidget {
    Q_OBJECT
public:
    explicit ProcessPanel(ScanController* controller, VisualizerWidget* viz, QWidget* parent = nullptr);

public slots:
    void onHistorySizeChanged(int size);

private slots:
    void onApplyAutoFilter();
    void onUndo();
    void onReset();
    void onEnableManualSelection(int state);
    void onDeleteSelected();

private:
    ScanController* m_controller;
    VisualizerWidget* m_viz;

    QDoubleSpinBox* m_radiusSpin;
    QDoubleSpinBox* m_minZSpin;
    QDoubleSpinBox* m_maxZSpin;
    
    QPushButton* m_btnAutoFilter;
    QPushButton* m_btnUndo;
    QPushButton* m_btnReset;

    QCheckBox* m_chkManualSelect;
    QPushButton* m_btnDeleteSelected;
};
