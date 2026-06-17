#include "ManualAlignDialog.hpp"
#include "../../controller/ScanController.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>

ManualAlignDialog::ManualAlignDialog(ScanController* ctrl, QWidget* parent)
    : QDialog(parent), m_ctrl(ctrl)
{
    setWindowTitle("Manual Coarse Alignment (Pre-Align)");
    setMinimumWidth(300);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* infoLabel = new QLabel("Change the values below to coarsely align the selected layer.\n"
                                   "It will move live in the 3D view.");
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    auto createSpinBox = [this](double min, double max, double step) {
        QDoubleSpinBox* spin = new QDoubleSpinBox(this);
        spin->setRange(min, max);
        spin->setSingleStep(step);
        spin->setValue(0.0);
        spin->setDecimals(1);
        connect(spin, &QDoubleSpinBox::valueChanged, this, &ManualAlignDialog::onValuesChanged);
        return spin;
    };

    // Kaydırma (Translation)
    QGroupBox* gbTrans = new QGroupBox("Translation (mm)", this);
    QFormLayout* lTrans = new QFormLayout(gbTrans);
    m_spnTx = createSpinBox(-500.0, 500.0, 5.0);
    m_spnTy = createSpinBox(-500.0, 500.0, 5.0);
    m_spnTz = createSpinBox(-500.0, 500.0, 5.0);
    lTrans->addRow("X Translation:", m_spnTx);
    lTrans->addRow("Y Translation:", m_spnTy);
    lTrans->addRow("Z Translation:", m_spnTz);
    mainLayout->addWidget(gbTrans);

    // Döndürme (Rotation)
    QGroupBox* gbRot = new QGroupBox("Rotation (Degrees)", this);
    QFormLayout* lRot = new QFormLayout(gbRot);
    m_spnRx = createSpinBox(-360.0, 360.0, 5.0);
    m_spnRy = createSpinBox(-360.0, 360.0, 5.0);
    m_spnRz = createSpinBox(-360.0, 360.0, 5.0);
    lRot->addRow("X Axis (Rx):", m_spnRx);
    lRot->addRow("Y Axis (Ry):", m_spnRy);
    lRot->addRow("Z Axis (Rz):", m_spnRz);
    mainLayout->addWidget(gbRot);

    // Butonlar
    QHBoxLayout* btnLayout = new QHBoxLayout;
    QPushButton* btnReset = new QPushButton("Reset", this);
    btnReset->setProperty("tier", "ghost");
    QPushButton* btnCancel = new QPushButton("Cancel", this);
    btnCancel->setProperty("tier", "ghost");
    QPushButton* btnApply = new QPushButton("Apply", this);
    btnApply->setProperty("tier", "primary");
    btnApply->setDefault(true);

    btnLayout->addWidget(btnReset);
    btnLayout->addStretch();
    btnLayout->addWidget(btnCancel);
    btnLayout->addWidget(btnApply);
    mainLayout->addLayout(btnLayout);

    connect(btnReset, &QPushButton::clicked, this, &ManualAlignDialog::onResetClicked);
    connect(btnCancel, &QPushButton::clicked, this, &ManualAlignDialog::reject);
    connect(btnApply, &QPushButton::clicked, this, &ManualAlignDialog::accept);

    connect(this, &QDialog::accepted, this, &ManualAlignDialog::onAccepted);
    connect(this, &QDialog::rejected, this, &ManualAlignDialog::onRejected);

    // Başlangıç yedeğini al
    if (m_ctrl) {
        m_ctrl->beginManualAlignment();
    }
}

ManualAlignDialog::~ManualAlignDialog()
{
}

void ManualAlignDialog::onValuesChanged()
{
    if (m_ctrl) {
        m_ctrl->updateManualAlignment(
            m_spnTx->value(), m_spnTy->value(), m_spnTz->value(),
            m_spnRx->value(), m_spnRy->value(), m_spnRz->value()
        );
    }
}

void ManualAlignDialog::onResetClicked()
{
    m_spnTx->blockSignals(true);
    m_spnTy->blockSignals(true);
    m_spnTz->blockSignals(true);
    m_spnRx->blockSignals(true);
    m_spnRy->blockSignals(true);
    m_spnRz->blockSignals(true);

    m_spnTx->setValue(0.0);
    m_spnTy->setValue(0.0);
    m_spnTz->setValue(0.0);
    m_spnRx->setValue(0.0);
    m_spnRy->setValue(0.0);
    m_spnRz->setValue(0.0);

    m_spnTx->blockSignals(false);
    m_spnTy->blockSignals(false);
    m_spnTz->blockSignals(false);
    m_spnRx->blockSignals(false);
    m_spnRy->blockSignals(false);
    m_spnRz->blockSignals(false);

    onValuesChanged();
}

void ManualAlignDialog::onAccepted()
{
    if (m_ctrl) {
        m_ctrl->commitManualAlignment();
    }
}

void ManualAlignDialog::onRejected()
{
    if (m_ctrl) {
        m_ctrl->cancelManualAlignment();
    }
}
