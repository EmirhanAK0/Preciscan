#include "SetupPanel.hpp"
#include "../../controller/ScanController.hpp"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QGroupBox>

SetupPanel::SetupPanel(ScanController* ctrl, QWidget* parent) 
    : QWidget(parent), m_ctrl(ctrl) 
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(10);

    auto* group = new QGroupBox("Lazer & Tarama Parametreleri", this);
    group->setStyleSheet("QGroupBox { color: #aaa; font-weight: bold; border: 1px solid #333; margin-top: 10px; padding-top: 15px; }");
    auto* form = new QFormLayout(group);

    // D_offset (Lazer-Eksen Mesafesi)
    m_dOffsetSpin = new QDoubleSpinBox(this);
    m_dOffsetSpin->setRange(10.0, 500.0);
    m_dOffsetSpin->setValue(66.0);
    m_dOffsetSpin->setSuffix(" mm");
    m_dOffsetSpin->setStyleSheet("background: #1a1a1a; color: #ccc; border: 1px solid #333; border-radius: 4px; padding: 4px;");
    connect(m_dOffsetSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SetupPanel::onDOffsetChanged);

    // Resolution (Adim Acisi)
    m_resSpin = new QDoubleSpinBox(this);
    m_resSpin->setRange(0.01, 10.0);
    m_resSpin->setValue(1.0);
    m_resSpin->setSingleStep(0.25);
    m_resSpin->setSuffix(" deg");
    m_resSpin->setStyleSheet("background: #1a1a1a; color: #ccc; border: 1px solid #333; border-radius: 4px; padding: 4px;");
    connect(m_resSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SetupPanel::onResolutionChanged);

    form->addRow("Lazer Ofseti (D):", m_dOffsetSpin);
    form->addRow("Tarama Cozunurlugu:", m_resSpin);

    layout->addWidget(group);
    layout->addStretch();
}

void SetupPanel::onDOffsetChanged(double val) {
    if (m_ctrl) m_ctrl->setDOffset(static_cast<float>(val));
}

void SetupPanel::onResolutionChanged(double val) {
    if (m_ctrl) m_ctrl->setResolution(static_cast<float>(val));
}