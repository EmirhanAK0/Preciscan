#pragma once

#include <QWidget>

class QSlider;
class QSpinBox;

class ExposureControl : public QWidget {
    Q_OBJECT

public:
    explicit ExposureControl(QWidget* parent = nullptr);

    void setValue(int microseconds);
    int value() const;

signals:
    void valueChanged(int microseconds);

public slots:
    void setEnabled(bool enabled);

private:
    QSlider* m_slider;
    QSpinBox* m_spinBox;
};
