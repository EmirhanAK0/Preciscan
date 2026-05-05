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

#include "../../controller/ScanController.hpp"

SetupPanel::SetupPanel(ScanController* ctrl, QWidget* parent)
    : QWidget(parent), m_ctrl(ctrl)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(10);

    auto* group = new QGroupBox("Lazer & Tarama Parametreleri", this);
    group->setStyleSheet(
        "QGroupBox {"
        "  color: #aaa;"
        "  font-weight: bold;"
        "  border: 1px solid #333;"
        "  margin-top: 10px;"
        "  padding-top: 15px;"
        "}"
    );

    auto* form = new QFormLayout(group);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    form->setFormAlignment(Qt::AlignTop);
    form->setHorizontalSpacing(14);
    form->setVerticalSpacing(10);

    m_dOffsetSpin = new QDoubleSpinBox(this);
    m_dOffsetSpin->setRange(10.0, 500.0);
    m_dOffsetSpin->setDecimals(2);
    m_dOffsetSpin->setSingleStep(0.5);
    m_dOffsetSpin->setValue(66.0);
    m_dOffsetSpin->setSuffix(" mm");
    applySpinStyle(m_dOffsetSpin);

    m_lOffsetSpin = new QDoubleSpinBox(this);
    m_lOffsetSpin->setRange(-100.0, 100.0);
    m_lOffsetSpin->setDecimals(2);
    m_lOffsetSpin->setSingleStep(0.5);
    m_lOffsetSpin->setValue(0.0);
    m_lOffsetSpin->setSuffix(" mm");
    applySpinStyle(m_lOffsetSpin);

    m_resSpin = new QDoubleSpinBox(this);
    m_resSpin->setRange(0.01, 10.0);
    m_resSpin->setDecimals(2);
    m_resSpin->setSingleStep(0.25);
    m_resSpin->setValue(1.0);
    m_resSpin->setSuffix(" deg");
    applySpinStyle(m_resSpin);

    m_profileRateSpin = new QSpinBox(this);
    m_profileRateSpin->setRange(1, 2000);
    m_profileRateSpin->setValue(100);
    m_profileRateSpin->setSuffix(" Hz");
    applySpinStyle(m_profileRateSpin);

    m_shutterSpin = new QSpinBox(this);
    m_shutterSpin->setRange(10, 10000);
    m_shutterSpin->setValue(100);
    m_shutterSpin->setSuffix(" us");
    applySpinStyle(m_shutterSpin);

    m_autoShutterCheck = new QCheckBox("Otomatik Pozlama", this);
    m_autoShutterCheck->setChecked(true);
    m_autoShutterCheck->setStyleSheet(
        "QCheckBox { color: #ccc; spacing: 8px; }"
        "QCheckBox::indicator { width: 16px; height: 16px; }"
    );

    m_measuringFieldCombo = new QComboBox(this);
    m_measuringFieldCombo->addItems({"large", "standard", "small"});
    applySpinStyle(m_measuringFieldCombo);

    m_pointsPerProfileCombo = new QComboBox(this);
    m_pointsPerProfileCombo->addItems({"1280", "640", "320", "160"});
    applySpinStyle(m_pointsPerProfileCombo);

    form->addRow("Lazer Ofseti (Z):", m_dOffsetSpin);
    form->addRow("Lateral Ofset (X):", m_lOffsetSpin);
    form->addRow("Tarama Cozunurlugu:", m_resSpin);
    form->addRow("Profiller / saniye:", m_profileRateSpin);
    form->addRow("Pozlama / Shutter:", m_shutterSpin);
    form->addRow("", m_autoShutterCheck);
    form->addRow("Olcum Alani:", m_measuringFieldCombo);
    form->addRow("Profil Nokta Sayisi:", m_pointsPerProfileCombo);

    m_readBtn = new QPushButton("Cihazdan Oku", this);
    m_applyBtn = new QPushButton("Uygula", this);

    m_readBtn->setStyleSheet(
        "QPushButton {"
        "  background: #1a1a1a;"
        "  color: #ccc;"
        "  border: 1px solid #333;"
        "  border-radius: 4px;"
        "  padding: 6px 10px;"
        "}"
        "QPushButton:hover {"
        "  border: 1px solid #555;"
        "}"
    );

    m_applyBtn->setStyleSheet(
        "QPushButton {"
        "  background: #0e5f2a;"
        "  color: white;"
        "  border: 1px solid #1faa59;"
        "  border-radius: 4px;"
        "  padding: 6px 10px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background: #117534;"
        "}"
    );

    auto* buttonRow = new QHBoxLayout();
    buttonRow->setSpacing(8);
    buttonRow->addWidget(m_readBtn);
    buttonRow->addWidget(m_applyBtn);

    layout->addWidget(group);

    // ── Tetik Modu ──────────────────────────────────────────────
    auto* trigGroup = new QGroupBox("Tetik Modu", this);
    trigGroup->setStyleSheet(
        "QGroupBox {"
        "  color: #5af;"
        "  font-weight: bold;"
        "  border: 1px solid #1a3a5a;"
        "  margin-top: 10px;"
        "  padding-top: 15px;"
        "}"
    );

    auto* trigForm = new QFormLayout(trigGroup);
    trigForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    trigForm->setHorizontalSpacing(14);
    trigForm->setVerticalSpacing(10);

    m_triggerModeCombo = new QComboBox(this);
    m_triggerModeCombo->addItem("Time-Based (Zamanlay\u0131c\u0131)",
                                QVariant::fromValue(static_cast<int>(ScanTriggerMode::TimeBased)));
    m_triggerModeCombo->addItem("Encoder Trigger (Enkoder)",
                                QVariant::fromValue(static_cast<int>(ScanTriggerMode::Encoder)));
    m_triggerModeCombo->addItem("External Trigger (Harici)",
                                QVariant::fromValue(static_cast<int>(ScanTriggerMode::ExternalTrigger)));
    applySpinStyle(m_triggerModeCombo);

    auto* trigInfoLabel = new QLabel(
        "<ul style='color:#777;font-size:9px;margin:0;padding-left:14px;'>"
        "<li><b style='color:#aaa'>Time-Based:</b> Dahili zamanlayici ile sabit adim</li>"
        "<li><b style='color:#5af'>Encoder:</b> MCU enkoder acisi her profilin konumunu belirler</li>"
        "<li><b style='color:#fa5'>External:</b> Harici dijital tetik sinyali ile tek profil</li>"
        "</ul>",
        this);
    trigInfoLabel->setWordWrap(true);

    trigForm->addRow("Mod:", m_triggerModeCombo);
    trigForm->addRow(trigInfoLabel);

    layout->addWidget(trigGroup);
    layout->addLayout(buttonRow);
    layout->addStretch();

    connect(m_applyBtn, &QPushButton::clicked, this, &SetupPanel::onApplyClicked);
    connect(m_readBtn, &QPushButton::clicked, this, &SetupPanel::onReadClicked);
}

void SetupPanel::applySpinStyle(QWidget* w)
{
    w->setStyleSheet(
        "background: #1a1a1a;"
        "color: #ccc;"
        "border: 1px solid #333;"
        "border-radius: 4px;"
        "padding: 4px;"
    );
}

void SetupPanel::onApplyClicked()
{
    if (!m_ctrl)
        return;

    const int profileRate = m_profileRateSpin->value();
    const int shutterUs = m_shutterSpin->value();
    const int periodUs = 1000000 / profileRate;

    if (shutterUs >= periodUs) {
        QMessageBox::warning(
            this,
            "Gecersiz Ayar",
            "Pozlama suresi, secilen profil hizinin periyodundan kucuk olmalidir."
        );
        return;
    }

    m_ctrl->setDOffset(static_cast<float>(m_dOffsetSpin->value()));
    m_ctrl->setLateralOffset(static_cast<float>(m_lOffsetSpin->value()));
    m_ctrl->setResolution(static_cast<float>(m_resSpin->value()));
    m_ctrl->setLaserProfileRate(m_profileRateSpin->value());
    m_ctrl->setLaserShutterUs(m_shutterSpin->value());
    m_ctrl->setLaserAutoShutter(m_autoShutterCheck->isChecked());
    m_ctrl->setLaserMeasuringField(m_measuringFieldCombo->currentText());
    m_ctrl->setLaserPointsPerProfile(m_pointsPerProfileCombo->currentText().toInt());

    // Tetik modunu uygula
    const int modeIdx = m_triggerModeCombo->currentData().toInt();
    m_ctrl->setTriggerMode(static_cast<ScanTriggerMode>(modeIdx));
}

void SetupPanel::onReadClicked()
{
    if (!m_ctrl)
        return;

    m_dOffsetSpin->setValue(m_ctrl->dOffset());
    m_lOffsetSpin->setValue(m_ctrl->lateralOffset());
    m_resSpin->setValue(m_ctrl->resolution());
    m_profileRateSpin->setValue(m_ctrl->laserProfileRate());
    m_shutterSpin->setValue(m_ctrl->laserShutterUs());
    m_autoShutterCheck->setChecked(m_ctrl->laserAutoShutter());
    m_measuringFieldCombo->setCurrentText(m_ctrl->laserMeasuringField());
    m_pointsPerProfileCombo->setCurrentText(QString::number(m_ctrl->laserPointsPerProfile()));

    // Tetik modunu oku
    const int modeIdx = static_cast<int>(m_ctrl->triggerMode());
    m_triggerModeCombo->setCurrentIndex(modeIdx);
}