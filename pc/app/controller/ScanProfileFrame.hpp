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
