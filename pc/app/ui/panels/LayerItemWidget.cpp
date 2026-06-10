#include "LayerItemWidget.hpp"

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

LayerItemWidget::LayerItemWidget(int layerId, const QString& name,
                                 int pointCount, QWidget* parent)
    : QWidget(parent), m_layerId(layerId)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(
        "LayerItemWidget { background-color: #2d2d30; border-radius: 4px; margin-bottom: 2px; }");

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(6, 4, 6, 4);
    layout->setSpacing(6);

    // Secim kutusu
    m_check = new QCheckBox(this);
    m_check->setChecked(true);

    // Isim alani (duzenlenebilir)
    m_nameEdit = new QLineEdit(name, this);
    m_nameEdit->setFixedWidth(100);
    m_nameEdit->setStyleSheet(
        "QLineEdit{background:#1a1a1a;color:#eee;border:1px solid #333;"
        "border-radius:2px;padding:1px 3px;font-size:9px;}");
    connect(m_nameEdit, &QLineEdit::editingFinished, this,
            [this]() { emit nameChanged(m_layerId, m_nameEdit->text()); });

    // Nokta sayisi etiketi
    auto* ptsLabel = new QLabel(QString("%1 pt").arg(pointCount), this);
    ptsLabel->setStyleSheet("color:#666;font-size:8px;");

    // Z-Offset spinbox
    auto* zLabel = new QLabel("Z:", this);
    zLabel->setStyleSheet("color:#888;font-size:8px;");

    m_zOffsetSpin = new QDoubleSpinBox(this);
    m_zOffsetSpin->setRange(-200.0, 200.0);
    m_zOffsetSpin->setValue(0.0);
    m_zOffsetSpin->setSuffix(" mm");
    m_zOffsetSpin->setDecimals(1);
    m_zOffsetSpin->setSingleStep(0.5);
    m_zOffsetSpin->setFixedWidth(75);
    m_zOffsetSpin->setStyleSheet(
        "QDoubleSpinBox{background:#1a1a1a;color:#5af;border:1px solid #333;"
        "border-radius:2px;padding:1px;font-size:8px;}");
    connect(m_zOffsetSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double v) { emit zOffsetChanged(m_layerId, (float)v); });

    // Sil butonu
    auto* btnDelete = new QPushButton("X", this);
    btnDelete->setFixedSize(20, 20);
    btnDelete->setCursor(Qt::PointingHandCursor);
    btnDelete->setStyleSheet(
        "QPushButton{background:#600;color:#fdd;border:none;border-radius:2px;font-size:10px;font-weight:bold;}"
        "QPushButton:hover{background:#900;}");
    connect(btnDelete, &QPushButton::clicked, this,
            [this]() { emit deleteRequested(m_layerId); });

    // Hizala butonu
    auto* btnAlign = new QPushButton("Hizala", this);
    btnAlign->setFixedSize(40, 20);
    btnAlign->setCursor(Qt::PointingHandCursor);
    btnAlign->setStyleSheet(
        "QPushButton{background:#3a3a3a;color:#ccc;border:1px solid #555;border-radius:2px;font-size:9px;}"
        "QPushButton:hover{background:#4a4a4a;color:#fff;}");
    connect(btnAlign, &QPushButton::clicked, this,
            [this]() { emit alignRequested(m_layerId); });

    layout->addWidget(m_check);
    layout->addWidget(m_nameEdit);
    layout->addWidget(ptsLabel);
    layout->addStretch();
    layout->addWidget(zLabel);
    layout->addWidget(m_zOffsetSpin);
    layout->addWidget(btnAlign);
    layout->addWidget(btnDelete);
}

bool LayerItemWidget::isSelected() const
{
    return m_check->isChecked();
}

QString LayerItemWidget::layerName() const
{
    return m_nameEdit->text();
}

float LayerItemWidget::zOffset() const
{
    return static_cast<float>(m_zOffsetSpin->value());
}

void LayerItemWidget::setZOffset(float mm)
{
    m_zOffsetSpin->setValue(static_cast<double>(mm));
}

void LayerItemWidget::setActive(bool active)
{
    m_isActive = active;
    if (m_isActive) {
        setStyleSheet("LayerItemWidget { background-color: #2e4c2e; border-radius: 4px; margin-bottom: 2px; border: 1px solid #2ecc71; }");
    } else {
        setStyleSheet("LayerItemWidget { background-color: #2d2d30; border-radius: 4px; margin-bottom: 2px; border: none; }");
    }
}

void LayerItemWidget::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);
    emit activated(m_layerId);
}
