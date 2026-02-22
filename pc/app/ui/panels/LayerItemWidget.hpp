#pragma once
#include <QWidget>

class QCheckBox;
class QLabel;

class LayerItemWidget : public QWidget {
    Q_OBJECT
public:
    explicit LayerItemWidget(int layerId, QWidget* parent = nullptr);

    bool isSelected() const;
    int layerId() const;

private:
    int m_layerId;
    QCheckBox* m_check;
};
