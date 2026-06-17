#include "SetupPanel.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>
#include <QInputDialog>
#include <QDialog>
#include <QDir>
#include "../../utils/AutoCalibrator.h"

#include "../../controller/ScanController.hpp"

SetupPanel::SetupPanel(ScanController* ctrl, QWidget* parent)
    : QWidget(parent), m_ctrl(ctrl)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(10);

    auto* group = new QGroupBox("Laser & Scan Parameters", this);

    auto* form = new QFormLayout(group);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    form->setFormAlignment(Qt::AlignTop);
    form->setHorizontalSpacing(14);
    form->setVerticalSpacing(10);

    m_dOffsetSpin = new QDoubleSpinBox(this);
    m_dOffsetSpin->setRange(10.0, 500.0);
    m_dOffsetSpin->setDecimals(2);
    m_dOffsetSpin->setSingleStep(0.5);
    m_dOffsetSpin->setValue(78.5);
    m_dOffsetSpin->setSuffix(" mm");
    applySpinStyle(m_dOffsetSpin);

    m_diamCalibBtn = new QPushButton("Dia. Calib.", this);
    m_diamCalibBtn->setToolTip(
        "Calibrates dOffset from a scan of a cylinder with a known diameter.\n"
        "Place the cylinder near the table center, scan it, then press this button.");

    auto* dOffsetLayout = new QHBoxLayout();
    dOffsetLayout->setContentsMargins(0, 0, 0, 0);
    dOffsetLayout->setSpacing(5);
    dOffsetLayout->addWidget(m_dOffsetSpin);
    dOffsetLayout->addWidget(m_diamCalibBtn);

    m_lOffsetSpin = new QDoubleSpinBox(this);
    m_lOffsetSpin->setRange(-100.0, 100.0);
    m_lOffsetSpin->setDecimals(2);
    m_lOffsetSpin->setSingleStep(0.5);
    m_lOffsetSpin->setValue(2.0);
    m_lOffsetSpin->setSuffix(" mm");
    applySpinStyle(m_lOffsetSpin);

    m_autoCalibBtn = new QPushButton("Auto Find", this);
    m_autoCalibBtn->setToolTip("Automatically computes the best lateral offset based on the last scan.");

    auto* lOffsetLayout = new QHBoxLayout();
    lOffsetLayout->setContentsMargins(0, 0, 0, 0);
    lOffsetLayout->setSpacing(5);
    lOffsetLayout->addWidget(m_lOffsetSpin);
    lOffsetLayout->addWidget(m_autoCalibBtn);

    // Yukseklik tabani (Z0): profil x'inden dunya Z'sine donusum ofseti.
    // Tabla yuzeyi z=0'a oturmuyorsa buradan duzeltilir.
    m_zBaseSpin = new QDoubleSpinBox(this);
    m_zBaseSpin->setRange(-200.0, 200.0);
    m_zBaseSpin->setDecimals(2);
    m_zBaseSpin->setSingleStep(0.1);
    m_zBaseSpin->setValue(3.5);
    m_zBaseSpin->setSuffix(" mm");
    applySpinStyle(m_zBaseSpin);

    m_autoZeroBtn = new QPushButton("Zero Base", this);
    m_autoZeroBtn->setToolTip(
        "Automatically finds the active layer's base plane (table surface)\n"
        "and sets it to z=0. The layer is re-projected from raw data.");

    auto* zBaseLayout = new QHBoxLayout();
    zBaseLayout->setContentsMargins(0, 0, 0, 0);
    zBaseLayout->setSpacing(5);
    zBaseLayout->addWidget(m_zBaseSpin);
    zBaseLayout->addWidget(m_autoZeroBtn);

    m_as5600ResSpin = new QDoubleSpinBox(this);
    m_as5600ResSpin->setRange(0.01, 10.0);
    m_as5600ResSpin->setDecimals(2);
    m_as5600ResSpin->setSingleStep(0.25);
    m_as5600ResSpin->setValue(1.0);
    m_as5600ResSpin->setSuffix(" deg");
    applySpinStyle(m_as5600ResSpin);

    m_stepResSpin = new QDoubleSpinBox(this);
    m_stepResSpin->setRange(0.01, 10.0);
    m_stepResSpin->setDecimals(2);
    m_stepResSpin->setSingleStep(0.25);
    m_stepResSpin->setValue(1.0);
    m_stepResSpin->setSuffix(" deg");
    applySpinStyle(m_stepResSpin);

    m_profileRateSpin = new QSpinBox(this);
    m_profileRateSpin->setRange(1, 2000);
    m_profileRateSpin->setValue(100);
    m_profileRateSpin->setSuffix(" Hz");
    applySpinStyle(m_profileRateSpin);

    m_measuringFieldCombo = new QComboBox(this);
    m_measuringFieldCombo->addItems({"large", "standard", "small"});
    applySpinStyle(m_measuringFieldCombo);

    m_pointsPerProfileCombo = new QComboBox(this);
    m_pointsPerProfileCombo->addItems({"1280", "640", "320", "160"});
    applySpinStyle(m_pointsPerProfileCombo);

    form->addRow("Laser Offset (Z):", dOffsetLayout);
    form->addRow("Lateral Offset (X):", lOffsetLayout);
    form->addRow("Height Base (Z0):", zBaseLayout);
    form->addRow("AS5600 Resolution:", m_as5600ResSpin);
    form->addRow("Step Resolution:", m_stepResSpin);
    form->addRow("Profile Rate (Hz):", m_profileRateSpin);
    form->addRow("Measuring Field:", m_measuringFieldCombo);
    form->addRow("Points per Profile:", m_pointsPerProfileCombo);

    m_readBtn = new QPushButton("Read from Device", this);   // ikincil (varsayilan tema)
    m_applyBtn = new QPushButton("Apply", this);
    m_applyBtn->setProperty("tier", "primary");              // birincil eylem

    layout->addWidget(group);

    // ── Tetik Modu ──────────────────────────────────────────────
    auto* trigGroup = new QGroupBox("Trigger Mode", this);

    auto* trigForm = new QFormLayout(trigGroup);
    trigForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    trigForm->setHorizontalSpacing(14);
    trigForm->setVerticalSpacing(10);

    m_triggerSourceCombo = new QComboBox(this);
    m_triggerSourceCombo->addItem("AS5600 (Magnetic Encoder)",
                                  QVariant::fromValue(static_cast<int>(TriggerSource::AS5600)));
    m_triggerSourceCombo->addItem("Step Angle (Motor Step)",
                                  QVariant::fromValue(static_cast<int>(TriggerSource::StepAngle)));
    m_triggerSourceCombo->setCurrentIndex(0); // Default to AS5600
    applySpinStyle(m_triggerSourceCombo);

    m_triggerModeCombo = new QComboBox(this);
    m_triggerModeCombo->addItem("Time-Based (Timer)",
                                QVariant::fromValue(static_cast<int>(ScanTriggerMode::TimeBased)));
    m_triggerModeCombo->addItem("Encoder Trigger",
                                QVariant::fromValue(static_cast<int>(ScanTriggerMode::Encoder)));
    m_triggerModeCombo->addItem("External Trigger",
                                QVariant::fromValue(static_cast<int>(ScanTriggerMode::ExternalTrigger)));
    m_triggerModeCombo->setCurrentIndex(1); // Default to Encoder
    applySpinStyle(m_triggerModeCombo);

    auto* trigInfoLabel = new QLabel(
        "<ul style='color:#777;font-size:9px;margin:0;padding-left:14px;'>"
        "<li><b style='color:#aaa'>Time-Based:</b> Fixed step via internal timer</li>"
        "<li><b style='color:#5af'>Encoder:</b> MCU encoder angle sets each profile's position</li>"
        "<li><b style='color:#fa5'>External:</b> One profile per external digital trigger</li>"
        "</ul>",
        this);
    trigInfoLabel->setWordWrap(true);

    trigForm->addRow("Source:", m_triggerSourceCombo);
    trigForm->addRow("Mode:", m_triggerModeCombo);
    trigForm->addRow(trigInfoLabel);

    layout->addWidget(trigGroup);

    // --- KALİBRASYON YÖNETİMİ GRUBU ---
    QGroupBox* calibGroup = new QGroupBox("3D Calibration Management", this);
    QVBoxLayout* calibLayout = new QVBoxLayout(calibGroup);

    m_calibMethodCombo = new QComboBox(this);
    m_calibMethodCombo->addItem("Method: PCL (Surface Scan)", 0);
    m_calibMethodCombo->addItem("Method: Mathematical (PCA)", 1);
    m_calibMethodCombo->setCurrentIndex(1); // Varsayılan Matematiksel (PCA)
    
    m_start3DCalibBtn = new QPushButton("Start New 3D Calibration", this);
    m_start3DCalibBtn->setProperty("tier", "primary");

    QHBoxLayout* calibControlLayout = new QHBoxLayout();
    m_calibProfileCombo = new QComboBox(this);
    m_loadCalibBtn = new QPushButton("Apply", this);
    m_clearCalibBtn = new QPushButton("Reset/Disable", this);
    
    calibControlLayout->addWidget(m_calibProfileCombo, 1);
    calibControlLayout->addWidget(m_loadCalibBtn);
    calibControlLayout->addWidget(m_clearCalibBtn);

    m_saveAsCalibBtn = new QPushButton("Save As...", this);

    calibLayout->addWidget(m_calibMethodCombo);
    calibLayout->addWidget(m_start3DCalibBtn);
    calibLayout->addLayout(calibControlLayout);
    calibLayout->addWidget(m_saveAsCalibBtn);
    
    layout->addWidget(calibGroup);
    // ----------------------------------

    auto* buttonRow = new QHBoxLayout();
    buttonRow->setSpacing(8);
    buttonRow->addStretch();
    buttonRow->addWidget(m_readBtn);
    buttonRow->addWidget(m_applyBtn);
    layout->addLayout(buttonRow);
    layout->addStretch();

    connect(m_applyBtn, &QPushButton::clicked, this, &SetupPanel::onApplyClicked);
    connect(m_readBtn, &QPushButton::clicked, this, &SetupPanel::onReadClicked);
    connect(m_autoCalibBtn, &QPushButton::clicked, this, [this]() {
        if (m_ctrl) m_ctrl->autoCalibrateOffsetAsync();
    });
    connect(m_autoZeroBtn, &QPushButton::clicked, this, [this]() {
        if (m_ctrl) m_ctrl->autoZeroBase();
    });
    connect(m_ctrl, &ScanController::zBaseOffsetChanged, this, [this](float mm) {
        m_zBaseSpin->blockSignals(true);
        m_zBaseSpin->setValue(mm);
        m_zBaseSpin->blockSignals(false);
    });
    connect(m_diamCalibBtn, &QPushButton::clicked, this, [this]() {
        if (!m_ctrl) return;

        // Bilinen cap + z-bandi giris dialogu
        QDialog dlg(this);
        dlg.setWindowTitle("Diameter Calibration (dOffset)");
        auto* dlgLayout = new QVBoxLayout(&dlg);

        auto* info = new QLabel(
            "Make the scan of the known-diameter cylinder the active layer.\n"
            "Select the Z band from the cylinder's STRAIGHT body (stay away\n"
            "from the base transition and the top surface).", &dlg);
        dlgLayout->addWidget(info);

        auto* formL = new QFormLayout();
        auto* diamSpin = new QDoubleSpinBox(&dlg);
        diamSpin->setRange(1.0, 300.0);
        diamSpin->setDecimals(3);
        diamSpin->setValue(30.0);
        diamSpin->setSuffix(" mm");

        auto* zMinSpin = new QDoubleSpinBox(&dlg);
        zMinSpin->setRange(-100.0, 300.0);
        zMinSpin->setDecimals(1);
        zMinSpin->setValue(10.0);
        zMinSpin->setSuffix(" mm");

        auto* zMaxSpin = new QDoubleSpinBox(&dlg);
        zMaxSpin->setRange(-100.0, 300.0);
        zMaxSpin->setDecimals(1);
        zMaxSpin->setValue(20.0);
        zMaxSpin->setSuffix(" mm");

        auto* maxRadiusSpin = new QDoubleSpinBox(&dlg);
        maxRadiusSpin->setRange(5.0, 200.0);
        maxRadiusSpin->setDecimals(1);
        maxRadiusSpin->setValue(40.0);
        maxRadiusSpin->setSuffix(" mm");
        maxRadiusSpin->setToolTip(
            "Points outside this radius from the axis are excluded from the fit.\n"
            "To remove the outer rim/ring around the table, choose a value larger\n"
            "than the cylinder radius but smaller than the rim radius.");

        formL->addRow("Known diameter (caliper):", diamSpin);
        formL->addRow("Band Z min:", zMinSpin);
        formL->addRow("Band Z max:", zMaxSpin);
        formL->addRow("Max radius (rim filter):", maxRadiusSpin);
        dlgLayout->addLayout(formL);

        auto* btnRow = new QHBoxLayout();
        auto* okBtn = new QPushButton("Calculate", &dlg);
        okBtn->setProperty("tier", "primary");
        auto* cancelBtn = new QPushButton("Cancel", &dlg);
        cancelBtn->setProperty("tier", "ghost");
        okBtn->setDefault(true);
        btnRow->addStretch();
        btnRow->addWidget(cancelBtn);
        btnRow->addWidget(okBtn);
        dlgLayout->addLayout(btnRow);

        connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
        connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

        if (dlg.exec() == QDialog::Accepted) {
            m_ctrl->calibrateDiameterAsync(
                static_cast<float>(diamSpin->value()),
                static_cast<float>(zMinSpin->value()),
                static_cast<float>(zMaxSpin->value()),
                static_cast<float>(maxRadiusSpin->value()));
        }
    });
    connect(m_ctrl, &ScanController::diameterCalibrationFinished, this,
            [this](bool success, QString report, float suggestedDOffset) {
        if (!success) {
            QMessageBox::warning(this, "Diameter Calibration", report);
            return;
        }
        const auto answer = QMessageBox::question(
            this, "Diameter Calibration",
            report + "\n\nApply the suggested dOffset?\n"
                     "(The active layer is re-projected from raw data.)",
            QMessageBox::Yes | QMessageBox::No);
        if (answer == QMessageBox::Yes) {
            m_dOffsetSpin->setValue(suggestedDOffset);
            m_ctrl->setDOffset(suggestedDOffset);
        }
    });
    connect(m_start3DCalibBtn, &QPushButton::clicked, this, [this]() {
        if (m_ctrl) m_ctrl->start3DCalibrationAsync(m_calibMethodCombo->currentIndex());
    });
    connect(m_ctrl, &ScanController::autoCalibrationFinished, this, [this](float bestOffset, double score) {
        QMessageBox::information(this, "Auto Calibration",
            QString("Calculation complete!\n\nBest lateral offset found: %1 mm\n\nThis value has been applied to the system.").arg(bestOffset));
        m_lOffsetSpin->setValue(bestOffset);
        // Degeri gercekten uygula: ham verisi olan aktif katman varsa
        // yeni offset ile otomatik olarak yeniden projekte edilir.
        m_ctrl->setLateralOffset(bestOffset);
    });
    connect(m_ctrl, &ScanController::calibration3DFinished, this, [this](bool success, QString report) {
        if (success) {
            QMessageBox::information(this, "3D PCL Calibration Successful", report);
        } else {
            QMessageBox::warning(this, "3D PCL Calibration Failed", report);
        }
    });

    // Kalibrasyon Yönetimi Butonları
    connect(m_loadCalibBtn, &QPushButton::clicked, this, [this]() {
        QString selected = m_calibProfileCombo->currentText();
        if (!selected.isEmpty()) {
            m_ctrl->updateActiveCalibration(selected);
        }
    });

    connect(m_clearCalibBtn, &QPushButton::clicked, this, [this]() {
        m_ctrl->disableCalibration();
        QMessageBox::information(this, "Reset", "Calibration disabled. All points reverted to their raw state.");
    });

    connect(m_saveAsCalibBtn, &QPushButton::clicked, this, [this]() {
        bool ok;
        QString text = QInputDialog::getText(this, tr("Save As"),
                                             tr("Calibration File Name:"), QLineEdit::Normal,
                                             "calibration_new", &ok);
        if (ok && !text.isEmpty()) {
            if (!text.endsWith(".json")) text += ".json";
            
            AutoCalibrator tempCalibrator; 
            tempCalibrator.setTransform(m_ctrl->getCalibrator()->getTransform());
            if (tempCalibrator.saveCalibration(text)) {
                QMessageBox::information(this, "Saved", text + " saved successfully.");
                refreshCalibrationList();
                int idx = m_calibProfileCombo->findText(text);
                if (idx >= 0) m_calibProfileCombo->setCurrentIndex(idx);
            } else {
                QMessageBox::warning(this, "Error", "File could not be saved!");
            }
        }
    });

    refreshCalibrationList();
}

void SetupPanel::refreshCalibrationList()
{
    m_calibProfileCombo->clear();
    QStringList files = AutoCalibrator::getAvailableCalibrations();
    m_calibProfileCombo->addItems(files);
}

void SetupPanel::applySpinStyle(QWidget* /*w*/)
{
    // Gorsel artik uygulama geneli temadan geliyor (QComboBox/QSpinBox/QDoubleSpinBox).
    // Geriye donuk uyumluluk icin imza korunuyor; govde bos.
}

void SetupPanel::onApplyClicked()
{
    if (!m_ctrl)
        return;

    m_ctrl->setDOffset(static_cast<float>(m_dOffsetSpin->value()));
    m_ctrl->setLateralOffset(static_cast<float>(m_lOffsetSpin->value()));
    m_ctrl->setZBaseOffset(static_cast<float>(m_zBaseSpin->value()));
    m_ctrl->setAs5600Resolution(static_cast<float>(m_as5600ResSpin->value()));
    m_ctrl->setStepResolution(static_cast<float>(m_stepResSpin->value()));
    m_ctrl->setLaserProfileRate(m_profileRateSpin->value());
    m_ctrl->setLaserMeasuringField(m_measuringFieldCombo->currentText());
    m_ctrl->setLaserPointsPerProfile(m_pointsPerProfileCombo->currentText().toInt());

    // Tetik modunu uygula
    const int modeIdx = m_triggerModeCombo->currentData().toInt();
    m_ctrl->setTriggerMode(static_cast<ScanTriggerMode>(modeIdx));

    // Tetik kaynagini uygula
    const int srcIdx = m_triggerSourceCombo->currentData().toInt();
    m_ctrl->setTriggerSource(static_cast<TriggerSource>(srcIdx));
}

void SetupPanel::onReadClicked()
{
    if (!m_ctrl)
        return;

    m_dOffsetSpin->setValue(m_ctrl->dOffset());
    m_lOffsetSpin->setValue(m_ctrl->lateralOffset());
    m_zBaseSpin->setValue(m_ctrl->zBaseOffset());
    m_as5600ResSpin->setValue(m_ctrl->as5600Resolution());
    m_stepResSpin->setValue(m_ctrl->stepResolution());
    m_profileRateSpin->setValue(m_ctrl->laserProfileRate());
    m_measuringFieldCombo->setCurrentText(m_ctrl->laserMeasuringField());
    m_pointsPerProfileCombo->setCurrentText(QString::number(m_ctrl->laserPointsPerProfile()));

    // Tetik modunu oku
    const int modeIdx = static_cast<int>(m_ctrl->triggerMode());
    m_triggerModeCombo->setCurrentIndex(modeIdx);

    // Tetik kaynagini oku
    const int srcIdx = static_cast<int>(m_ctrl->triggerSource());
    m_triggerSourceCombo->setCurrentIndex(srcIdx);
}