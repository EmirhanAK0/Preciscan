#include "SpeedControl.hpp"
#include <QHBoxLayout>
#include <QSlider>
#include <QSpinBox>
#include <QLabel>

SpeedControl::SpeedControl(QWidget* parent) : QWidget(parent) {
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(new QLabel("Speed (RPM):", this));

    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setRange(0, 3000); // Example range

    m_spinBox = new QSpinBox(this);
    m_spinBox->setRange(0, 3000);
    m_spinBox->setSuffix(" RPM");

    layout->addWidget(m_slider);
    layout->addWidget(m_spinBox);

    // Sync slider and spinbox
    connect(m_slider, &QSlider::valueChanged, m_spinBox, &QSpinBox::setValue);
    connect(m_spinBox, &QSpinBox::valueChanged, m_slider, &QSlider::setValue);

    // Emit signal
    connect(m_spinBox, &QSpinBox::valueChanged, this, &SpeedControl::valueChanged);
}

void SpeedControl::setValue(int rpm) {
    m_spinBox->setValue(rpm);
}

int SpeedControl::value() const {
    return m_spinBox->value();
}

void SpeedControl::setEnabled(bool enabled) {
    QWidget::setEnabled(enabled);
}
