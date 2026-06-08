#pragma once
#include <QWidget>
#include <QVector>
#include <QVector3D>

class QCheckBox;
class QLabel;
class QLineEdit;
class QDoubleSpinBox;

/// Bir tarama katmanının nokta bulutu ve meta verilerini tutar.
struct ScanLayer {
    QString           name;
    QVector<QVector3D> points;
    float             zOffsetMm = 0.0f;  ///< Birleştirme sırasında Z'ye eklenecek offset
};

/// Katman listesindeki her satırın widget'ı.
/// İsim düzenleyici, Z-offset spinbox, seçim kutusu ve silme butonu içerir.
class LayerItemWidget : public QWidget {
    Q_OBJECT
public:
    explicit LayerItemWidget(int layerId, const QString& name,
                             int pointCount, QWidget* parent = nullptr);

    bool    isSelected() const;
    bool    isActive()   const { return m_isActive; }
    void    setActive(bool active);
    
    int     layerId()    const { return m_layerId; }
    QString layerName()  const;
    float   zOffset()    const;

    void setZOffset(float mm);

signals:
    void deleteRequested(int layerId);
    void nameChanged(int layerId, const QString& newName);
    void zOffsetChanged(int layerId, float mm);
    void activated(int layerId);

protected:
    void mousePressEvent(QMouseEvent* event) override;

private:
    int            m_layerId;
    bool           m_isActive = false;
    QCheckBox*     m_check;
    QLineEdit*     m_nameEdit;
    QDoubleSpinBox* m_zOffsetSpin;
};
