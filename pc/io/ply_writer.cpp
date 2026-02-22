#include "ply_writer.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

namespace io {

bool writePLY(const QString& path, const QVector<QVector3D>& points) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "PLY dosyasi acilamadi:" << path;
        return false;
    }

    QTextStream out(&file);
    
    // Header
    out << "ply\n";
    out << "format ascii 1.0\n";
    out << "element vertex " << points.size() << "\n";
    out << "property float x\n";
    out << "property float y\n";
    out << "property float z\n";
    out << "end_header\n";

    // Data
    for (const auto& p : points) {
        out << p.x() << " " << p.y() << " " << p.z() << "\n";
    }

    file.close();
    return true;
}

} // namespace io
