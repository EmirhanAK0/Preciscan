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
    QDoubleSpinBox* m_lOffsetSpin = nullptr;
    QDoubleSpinBox* m_as5600ResSpin = nullptr;
    QDoubleSpinBox* m_stepResSpin = nullptr;

    QSpinBox* m_profileRateSpin = nullptr;

    QComboBox* m_measuringFieldCombo = nullptr;
    QComboBox* m_pointsPerProfileCombo = nullptr;
    QComboBox* m_triggerModeCombo = nullptr;
    QComboBox* m_triggerSourceCombo = nullptr;

    QPushButton* m_readBtn = nullptr;
    QPushButton* m_applyBtn = nullptr;
    QPushButton* m_autoCalibBtn = nullptr;
    QPushButton* m_start3DCalibBtn = nullptr;
    QComboBox* m_calibMethodCombo = nullptr;

    // Kalibrasyon Yönetimi
    QComboBox* m_calibProfileCombo = nullptr;
    QPushButton* m_loadCalibBtn = nullptr;
    QPushButton* m_clearCalibBtn = nullptr;
    QPushButton* m_saveAsCalibBtn = nullptr;

private:
    void refreshCalibrationList();
};