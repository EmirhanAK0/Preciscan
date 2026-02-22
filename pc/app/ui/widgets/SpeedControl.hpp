#pragma once

#include <QWidget>

class QSlider;
class QSpinBox;

class SpeedControl : public QWidget {
    Q_OBJECT

public:
    explicit SpeedControl(QWidget* parent = nullptr);

    void setValue(int rpm);
    int value() const;

signals:
    void valueChanged(int rpm);

public slots:
    void setEnabled(bool enabled);

private:
    QSlider* m_slider;
    QSpinBox* m_spinBox;
};
