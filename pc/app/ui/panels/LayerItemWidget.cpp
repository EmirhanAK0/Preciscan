#include "LayerItemWidget.hpp"
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>

LayerItemWidget::LayerItemWidget(int layerId, QWidget* parent)
    : QWidget(parent), m_layerId(layerId)
{
    // Kart gibi görünmesi için arka plan boyamasýný aktif et
    setAttribute(Qt::WA_StyledBackground, true);
    // Bu widget'a özel stil (Listenin içinde þerit gibi duracak)
    setStyleSheet("LayerItemWidget { background-color: #2d2d30; border-radius: 4px; margin-bottom: 4px; }");

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 8, 10, 8); // Ýç boþluklar
    layout->setSpacing(15);

    // 1. Seçim Kutusu (Görünürlük yerine kullanýlabilir)
    m_check = new QCheckBox(this);
    // Checkbox'ý biraz büyütelim veya özelleþtirelim istersen QSS ile yapýlýr

    // 2. Katman Ýsmi
    auto* label = new QLabel(QString("Scan Layer #%1").arg(layerId), this);
    label->setStyleSheet("font-weight: bold; color: #f0f0f0;");

    // 3. Sil Butonu (Metin tabanlý: [X])
    auto* btnDelete = new QPushButton("?", this);
    btnDelete->setFixedSize(24, 24);
    btnDelete->setCursor(Qt::PointingHandCursor);
    btnDelete->setStyleSheet(
        "QPushButton { background: transparent; color: #cc5555; font-weight: bold; font-size: 14px; }"
        "QPushButton:hover { background: #3e3e42; color: #ff6666; }"
    );

    // Silme fonksiyonu eklenecekse connect buraya...

    layout->addWidget(m_check);
    layout->addWidget(label);
    layout->addStretch(); // Araya yay koyup sil butonunu saða itiyoruz
    layout->addWidget(btnDelete);

    setLayout(layout);
}

bool LayerItemWidget::isSelected() const {
    return m_check->isChecked();
}

int LayerItemWidget::layerId() const {
    return m_layerId;
}