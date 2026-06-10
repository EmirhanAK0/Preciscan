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
    setWindowTitle("Manuel Kaba Hizalama (Pre-Align)");
    setMinimumWidth(300);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* infoLabel = new QLabel("Seçili katmanı (Layer) kaba hizalamak için aşağıdaki değerleri değiştirin.\n"
                                   "3D ekranda canlı olarak hareket edecektir.");
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
    QGroupBox* gbTrans = new QGroupBox("Kaydırma (mm)", this);
    QFormLayout* lTrans = new QFormLayout(gbTrans);
    m_spnTx = createSpinBox(-500.0, 500.0, 5.0);
    m_spnTy = createSpinBox(-500.0, 500.0, 5.0);
    m_spnTz = createSpinBox(-500.0, 500.0, 5.0);
    lTrans->addRow("X Kaydırma:", m_spnTx);
    lTrans->addRow("Y Kaydırma:", m_spnTy);
    lTrans->addRow("Z Kaydırma:", m_spnTz);
    mainLayout->addWidget(gbTrans);

    // Döndürme (Rotation)
    QGroupBox* gbRot = new QGroupBox("Döndürme (Derece)", this);
    QFormLayout* lRot = new QFormLayout(gbRot);
    m_spnRx = createSpinBox(-360.0, 360.0, 5.0);
    m_spnRy = createSpinBox(-360.0, 360.0, 5.0);
    m_spnRz = createSpinBox(-360.0, 360.0, 5.0);
    lRot->addRow("X Ekseni (Rx):", m_spnRx);
    lRot->addRow("Y Ekseni (Ry):", m_spnRy);
    lRot->addRow("Z Ekseni (Rz):", m_spnRz);
    mainLayout->addWidget(gbRot);

    // Butonlar
    QHBoxLayout* btnLayout = new QHBoxLayout;
    QPushButton* btnReset = new QPushButton("Sıfırla", this);
    QPushButton* btnCancel = new QPushButton("İptal", this);
    QPushButton* btnApply = new QPushButton("Uygula", this);
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
