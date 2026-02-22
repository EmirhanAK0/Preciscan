#pragma once
#include <QString>
#include <QVector>
#include <QVector3D>

namespace io {

// .ply (Polygon File Format) dosyasi yazar (ASCII).
// Bulut islemleri ve gosterimi icin standart format.
bool writePLY(const QString& path, const QVector<QVector3D>& points);

} // namespace io
