#pragma once

#include <QWidget>

#include "../../controller/ScanController.hpp"

class ScanController;
class QDoubleSpinBox;
class QSpinBox;
class QCheckBox;
class QComboBox;
class QPushButton;
class QLabel;

class SetupPanel : public QWidget {
    Q_OBJECT

public:
    explicit SetupPanel(ScanController* ctrl, QWidget* parent = nullptr);

private slots:
    void onApplyClicked();
    void onReadClicked();

private:
    void applySpinStyle(QWidget* w);

private:
    ScanController* m_ctrl = nullptr;

    QDoubleSpinBox* m_dOffsetSpin = nullptr;
    QDoubleSpinBox* m_resSpin = nullptr;

    QSpinBox* m_profileRateSpin = nullptr;
    QSpinBox* m_shutterSpin = nullptr;

    QCheckBox* m_autoShutterCheck = nullptr;

    QComboBox* m_measuringFieldCombo = nullptr;
    QComboBox* m_pointsPerProfileCombo = nullptr;
    QComboBox* m_triggerModeCombo = nullptr;

    QPushButton* m_applyBtn = nullptr;
    QPushButton* m_readBtn = nullptr;
};