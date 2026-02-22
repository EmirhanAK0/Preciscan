#pragma once
#include <QWidget>

class ScanController;
class QDoubleSpinBox;

class SetupPanel : public QWidget {
    Q_OBJECT
public:
    explicit SetupPanel(ScanController* ctrl, QWidget* parent = nullptr);

private slots:
    void onDOffsetChanged(double val);
    void onResolutionChanged(double val);

private:
    ScanController* m_ctrl;
    QDoubleSpinBox* m_dOffsetSpin;
    QDoubleSpinBox* m_resSpin;
};