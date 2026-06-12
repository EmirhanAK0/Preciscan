#pragma once

#include <QPointF>
#include <QVector>
#include <QString>

/// Tarama yonu
enum class ScanDirection {
    Clockwise,
    CounterClockwise
};

inline QString scanDirectionName(ScanDirection d)
{
    return (d == ScanDirection::Clockwise) ? "CW" : "CCW";
}

/// Tek bir tarama profilini temsil eden veri cercevesi.
/// VisualizerWidget ve ScanPanel tarafindan kullanilir.
struct ScanProfileFrame
{
    QVector<QPointF> profile;     ///< 2D lazer profil noktaları (X, Z)
    float thetaDegree = 0.0f;    ///< Döner tabla açısı (derece)
    int layerIndex = 0;          ///< Katman numarası (multi-layer tarama için)
    ScanDirection direction = ScanDirection::Clockwise;
};

/// Ham tarama ornegi: projeksiyon ONCESI (aci, profil) cifti.
/// Nokta bulutu bu veriden uretildigi icin, dOffset/lateral offset gibi
/// projeksiyon parametreleri sonradan degistiginde bulut yeniden
/// projeksiyonla kurtarilabilir (yeniden tarama gerekmez).
struct RawProfileSample
{
    float thetaDegree = 0.0f;     ///< Tetik anindaki tabla acisi (derece)
    QVector<QPointF> profile;     ///< Lazer profil noktalari (X, Z) — sensor duzleminde
};
