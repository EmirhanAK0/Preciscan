#include "ExposureControl.hpp"
#include <QHBoxLayout>
#include <QSlider>
#include <QSpinBox>
#include <QLabel>

ExposureControl::ExposureControl(QWidget* parent) : QWidget(parent) {
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(new QLabel("Exposure (µs):", this));

    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setRange(100, 20000); // Example range

    m_spinBox = new QSpinBox(this);
    m_spinBox->setRange(100, 20000);
    m_spinBox->setSuffix(" µs");

    layout->addWidget(m_slider);
    layout->addWidget(m_spinBox);

    connect(m_slider, &QSlider::valueChanged, m_spinBox, &QSpinBox::setValue);
    connect(m_spinBox, &QSpinBox::valueChanged, m_slider, &QSlider::setValue);

    connect(m_spinBox, &QSpinBox::valueChanged, this, &ExposureControl::valueChanged);
}

void ExposureControl::setValue(int microseconds) {
    m_spinBox->setValue(microseconds);
}

int ExposureControl::value() const {
    return m_spinBox->value();
}

void ExposureControl::setEnabled(bool enabled) {
    QWidget::setEnabled(enabled);
}
