#pragma once
#include <QString>
#include <QVector>

#include <QVector3D>

namespace io
{

// .ply (Polygon File Format) dosyasi yazar (ASCII).
// Bulut islemleri ve gosterimi icin standart format.
bool writePLY(const QString& path, const QVector<QVector3D>& points);
bool readPLY(const QString& path, QVector<QVector3D>& points);

// Normalli PLY yazar (nx, ny, nz alanlari ile). MeshLab'da normal hesaplama
// adimini atlayip dogrudan Poisson mesh kurmayi saglar.
// normals.size() != points.size() ise normalsiz yazima duser.
bool writePLYWithNormals(const QString& path,
                         const QVector<QVector3D>& points,
                         const QVector<QVector3D>& normals);

} // namespace io
