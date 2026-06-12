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
#include <QDir>
#include "../../utils/AutoCalibrator.h"

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
    m_dOffsetSpin->setValue(78.5);
    m_dOffsetSpin->setSuffix(" mm");
    applySpinStyle(m_dOffsetSpin);

    m_lOffsetSpin = new QDoubleSpinBox(this);
    m_lOffsetSpin->setRange(-100.0, 100.0);
    m_lOffsetSpin->setDecimals(2);
    m_lOffsetSpin->setSingleStep(0.5);
    m_lOffsetSpin->setValue(2.0);
    m_lOffsetSpin->setSuffix(" mm");
    applySpinStyle(m_lOffsetSpin);

    m_autoCalibBtn = new QPushButton("Oto. Bul", this);
    m_autoCalibBtn->setToolTip("Son taramayi baz alip en iyi lateral offseti otomatik hesaplar.");
    m_autoCalibBtn->setStyleSheet(
        "QPushButton {"
        "  background: #2a4a6a;"
        "  color: white;"
        "  border: 1px solid #3a5a7a;"
        "  border-radius: 4px;"
        "  padding: 2px 5px;"
        "}"
        "QPushButton:hover { background: #3a5a7a; }"
    );
    
    auto* lOffsetLayout = new QHBoxLayout();
    lOffsetLayout->setContentsMargins(0, 0, 0, 0);
    lOffsetLayout->setSpacing(5);
    lOffsetLayout->addWidget(m_lOffsetSpin);
    lOffsetLayout->addWidget(m_autoCalibBtn);

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

    form->addRow("Lazer Ofseti (Z):", m_dOffsetSpin);
    form->addRow("Lateral Ofset (X):", lOffsetLayout);
    form->addRow("AS5600 Çözünürlüğü:", m_as5600ResSpin);
    form->addRow("Step Çözünürlüğü:", m_stepResSpin);
    form->addRow("Profil Hızı (Hz):", m_profileRateSpin);
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

    m_triggerSourceCombo = new QComboBox(this);
    m_triggerSourceCombo->addItem("AS5600 (Manyetik Enkoder)",
                                  QVariant::fromValue(static_cast<int>(TriggerSource::AS5600)));
    m_triggerSourceCombo->addItem("Step Açısı (Motor Adımı)",
                                  QVariant::fromValue(static_cast<int>(TriggerSource::StepAngle)));
    m_triggerSourceCombo->setCurrentIndex(0); // Default to AS5600
    applySpinStyle(m_triggerSourceCombo);

    m_triggerModeCombo = new QComboBox(this);
    m_triggerModeCombo->addItem("Time-Based (Zamanlayıcı)",
                                QVariant::fromValue(static_cast<int>(ScanTriggerMode::TimeBased)));
    m_triggerModeCombo->addItem("Encoder Trigger (Enkoder)",
                                QVariant::fromValue(static_cast<int>(ScanTriggerMode::Encoder)));
    m_triggerModeCombo->addItem("External Trigger (Harici)",
                                QVariant::fromValue(static_cast<int>(ScanTriggerMode::ExternalTrigger)));
    m_triggerModeCombo->setCurrentIndex(1); // Default to Encoder
    applySpinStyle(m_triggerModeCombo);

    auto* trigInfoLabel = new QLabel(
        "<ul style='color:#777;font-size:9px;margin:0;padding-left:14px;'>"
        "<li><b style='color:#aaa'>Time-Based:</b> Dahili zamanlayici ile sabit adim</li>"
        "<li><b style='color:#5af'>Encoder:</b> MCU enkoder acisi her profilin konumunu belirler</li>"
        "<li><b style='color:#fa5'>External:</b> Harici dijital tetik sinyali ile tek profil</li>"
        "</ul>",
        this);
    trigInfoLabel->setWordWrap(true);

    trigForm->addRow("Kaynak:", m_triggerSourceCombo);
    trigForm->addRow("Mod:", m_triggerModeCombo);
    trigForm->addRow(trigInfoLabel);

    layout->addWidget(trigGroup);

    // --- KALİBRASYON YÖNETİMİ GRUBU ---
    QGroupBox* calibGroup = new QGroupBox("3D Kalibrasyon Yönetimi", this);
    QVBoxLayout* calibLayout = new QVBoxLayout(calibGroup);

    m_calibMethodCombo = new QComboBox(this);
    m_calibMethodCombo->addItem("Yöntem: PCL (Yüzey Taraması)", 0);
    m_calibMethodCombo->addItem("Yöntem: Matematiksel (PCA)", 1);
    m_calibMethodCombo->setCurrentIndex(1); // Varsayılan Matematiksel (PCA)
    
    m_start3DCalibBtn = new QPushButton("Yeni 3D Kalibrasyon Başlat", this);
    m_start3DCalibBtn->setStyleSheet("background-color: #8B0000; color: white; font-weight: bold;");

    QHBoxLayout* calibControlLayout = new QHBoxLayout();
    m_calibProfileCombo = new QComboBox(this);
    m_loadCalibBtn = new QPushButton("Uygula", this);
    m_clearCalibBtn = new QPushButton("Sıfırla/Kapat", this);
    
    calibControlLayout->addWidget(m_calibProfileCombo, 1);
    calibControlLayout->addWidget(m_loadCalibBtn);
    calibControlLayout->addWidget(m_clearCalibBtn);

    m_saveAsCalibBtn = new QPushButton("Farklı Kaydet...", this);

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
    connect(m_start3DCalibBtn, &QPushButton::clicked, this, [this]() {
        if (m_ctrl) m_ctrl->start3DCalibrationAsync(m_calibMethodCombo->currentIndex());
    });
    connect(m_ctrl, &ScanController::autoCalibrationFinished, this, [this](float bestOffset, double score) {
        QMessageBox::information(this, "Otomatik Kalibrasyon", 
            QString("Hesaplama tamamlandi!\n\nBulunan en iyi Lateral Offset: %1 mm\n\nBu deger sisteme uygulandi.").arg(bestOffset));
        m_lOffsetSpin->setValue(bestOffset);
    });
    connect(m_ctrl, &ScanController::calibration3DFinished, this, [this](bool success, QString report) {
        if (success) {
            QMessageBox::information(this, "3D PCL Kalibrasyonu Başarılı", report);
        } else {
            QMessageBox::warning(this, "3D PCL Kalibrasyonu Başarısız", report);
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
        QMessageBox::information(this, "Sıfırlandı", "Kalibrasyon devre dışı bırakıldı. Tüm noktalar ham hallerine döndürüldü.");
    });

    connect(m_saveAsCalibBtn, &QPushButton::clicked, this, [this]() {
        bool ok;
        QString text = QInputDialog::getText(this, tr("Farklı Kaydet"),
                                             tr("Kalibrasyon Dosya İsmi:"), QLineEdit::Normal,
                                             "calibration_new", &ok);
        if (ok && !text.isEmpty()) {
            if (!text.endsWith(".json")) text += ".json";
            
            AutoCalibrator tempCalibrator; 
            tempCalibrator.setTransform(m_ctrl->getCalibrator()->getTransform());
            if (tempCalibrator.saveCalibration(text)) {
                QMessageBox::information(this, "Kaydedildi", text + " basariyla kaydedildi.");
                refreshCalibrationList();
                int idx = m_calibProfileCombo->findText(text);
                if (idx >= 0) m_calibProfileCombo->setCurrentIndex(idx);
            } else {
                QMessageBox::warning(this, "Hata", "Dosya kaydedilemedi!");
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

    m_ctrl->setDOffset(static_cast<float>(m_dOffsetSpin->value()));
    m_ctrl->setLateralOffset(static_cast<float>(m_lOffsetSpin->value()));
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