#include "InfoTipButton.hpp"

InfoTipButton::InfoTipButton(const QString& tooltipText, QWidget* parent)
    : QToolButton(parent) {
    setText("?");
    setToolTip(tooltipText);
    setFixedSize(20, 20);
    // Optional: Style it to look like a small circle or helpful icon
    setStyleSheet("QToolButton { border-radius: 10px; background-color: #ddd; font-weight: bold; }");
}
