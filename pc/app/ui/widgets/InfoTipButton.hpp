#pragma once

#include <QToolButton>

class InfoTipButton : public QToolButton {
    Q_OBJECT

public:
    explicit InfoTipButton(const QString& tooltipText, QWidget* parent = nullptr);
};
